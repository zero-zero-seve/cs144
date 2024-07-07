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
        // ��һ�����и��Է�����syn����
        TCPSenderMessage msg{isn_, true, nullptr, input_.reader().is_finished(), input_.reader().has_error()};
        next_usefulSeq += 1;
        transmit(msg);
        return;
    }
    // ������ڿ�ס���кſռ仹��δʹ�����к�
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
        // ��ack��ת��Ϊ�������к�
        uint64_t temp = msg.ackno.unwrap(isn_, recent_ackSeq);
        // ������ȷ�ϣ�����Ӷ������Ƴ�
        // TODO: ��һ����������û�еõ�ȷ��ack�������ڽ��ܷ�����ack
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
        // ��ʱ�����ش�
        auto it = flight_msg.begin();
        if (it != flight_msg.end()) {
            transmit(it->second);
        }
        // TODO: ������ڴ�С��0������������������
        consec_retransmission_numbers += 1;
        remaining_time = (1UL << consec_retransmission_numbers) * initial_RTO_ms_;
    } else {
        remaining_time -= ms_since_last_tick;
    }
}
