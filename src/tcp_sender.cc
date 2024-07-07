#include "tcp_sender.hh"

#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const {
    auto it = flight_msg.begin();
    uint64_t sum = 0;
    for (it; it != flight_msg.end(); it++) {
        sum += it->second.sequence_length();
    }
    return sum;
}

uint64_t TCPSender::consecutive_retransmissions() const { return consec_retransmission_numbers; }

void TCPSender::push(const TransmitFunction& transmit) {
    if (!isSyn) {
        // 第一次运行给对方传输syn报文
        TCPSenderMessage msg{isn_, true, nullptr, input_.reader().is_finished(), input_.reader().has_error()};
        next_usefulSeq += 1;
        transmit(msg);
        return;
    }
    // 如果窗口框住序列号空间还有未使用序列号
    if (recent_ackSeq + windowSize > next_usefulSeq) {
        uint64_t len = recent_ackSeq + windowSize - next_usefulSeq;
        std::string out = "";
        while (len > TCPConfig::MAX_PAYLOAD_SIZE && !isFin) {
            read(input_.reader(), TCPConfig::MAX_PAYLOAD_SIZE, out);
            TCPSenderMessage msg{Wrap32::wrap(next_usefulSeq, isn_), false, out,
                                 input_.reader().is_finished(), input_.reader().has_error()};
            flight_msg.push_back(std::pair<uint64_t, TCPSenderMessage>(next_usefulSeq, msg));
            transmit(msg);
            if (isStoped) {
                isStoped = false;
            }
            len = len - out.length() - input_.reader().is_finished();
            next_usefulSeq = next_usefulSeq + out.length() + input_.reader().is_finished();
            if (input_.reader().is_finished()) {
                isFin = true;
            }
        }
        if (len > 0 && !isFin) {
            read(input_.reader(), len, out);
            len = len - out.length();
            TCPSenderMessage msg{Wrap32::wrap(next_usefulSeq, isn_), false, out, false,
                                 input_.reader().has_error()};
            if (input_.reader().is_finished()) {
                if (len >= 1) {
                    msg.FIN = true;
                    isFin = true;
                }
            }
            flight_msg.push_back(std::pair<uint64_t, TCPSenderMessage>(next_usefulSeq, msg));
            transmit(msg);
            if (isStoped) {
                isStoped = false;
            }
            next_usefulSeq = out.length() + msg.FIN;
        }
        return;
    } else {
        TCPSenderMessage msg = make_empty_message();
        transmit(msg);
        return;
    }
}

TCPSenderMessage TCPSender::make_empty_message() const {
    TCPSenderMessage msg{Wrap32::wrap(next_usefulSeq, isn_), false, nullptr, false,
                         input_.reader().has_error()};
    return msg;
}

void TCPSender::receive(const TCPReceiverMessage& msg) {
    if (msg.RST) {
        input_.set_error();
    } else if (msg.ackno != std::nullopt) {
        // 把ack号转化为绝对序列号
        uint64_t temp = msg.ackno.unwrap(isn_, recent_ackSeq);
        // 尽可能确认，而后从队列中移除
        // TODO: 第一个发出但是没有得到确认ack并不等于接受方返回ack
        auto it = flight_msg.begin();
        for (; it != flight_msg.end(); it++) {
            if (it->first + it->second.sequence_length() <= temp) {
                flight_msg.pop_front();
                remaining_time = initial_RTO_ms_;
                isStoped = true;
                consec_retransmission_numbers = 0;
                first_unacked_seq = it->first + it->second.sequence_length();
            } else {
                break;
            }
        }
        if (it == flight_msg.end()) {
            isStoped = false;
        }
        if (recent_ackSeq < temp) {
            recent_ackSeq = temp;
        }
        windowSize = msg.window_size;
    }
}

void TCPSender::tick(uint64_t ms_since_last_tick, const TransmitFunction& transmit) {
    if (isStoped) {
        return;
    }
    if (ms_since_last_tick >= remaining_time) {
        remaining_time = 0;
        // 超时进行重传
        auto it = flight_msg.begin();
        if (it != flight_msg.end()) {
            transmit(it->second);
        }
        // TODO: 如果窗口大小非0，跟踪连续次数？？
        consec_retransmission_numbers += 1;
        remaining_time = (1UL << consec_retransmission_numbers) * initial_RTO_ms_;
    } else {
        remaining_time -= ms_since_last_tick;
    }
}
