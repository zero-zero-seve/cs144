#include "tcp_receiver.hh"

#include "wrapping_integers.hh"
using namespace std;

void TCPReceiver::receive(TCPSenderMessage message) {
    // �յ���Ϣ���鿴syn��־λ�Ƿ���λ�������λ��˵��seqΪISN
    bool is_last_substring = false;
    uint64_t first_index;
    if (message.SYN) {
        ISN = message.seqno;
        isGetISN = true;
    }
    if (message.FIN) {
        is_last_substring = true;
    }
    if (message.RST) {
        error_ = true;
        reassembler_.reader().set_error();
    }
    if (isGetISN) {
        first_index = message.seqno.unwrap(ISN, reassembler_.writer().bytes_pushed());
        if (!message.SYN) first_index -= 1;
        reassembler_.insert(first_index, message.payload, is_last_substring);
    }
}

TCPReceiverMessage TCPReceiver::send() const {
    uint16_t avai_capacity = (uint16_t)reassembler_.writer().available_capacity();
    if (reassembler_.writer().available_capacity() > (uint64_t)65535) {
        avai_capacity = UINT16_MAX;
    }
    if (isGetISN) {
        Wrap32 ackno =
            Wrap32::wrap(reassembler_.writer().bytes_pushed() + 1 + reassembler_.writer().is_closed(), ISN);
        TCPReceiverMessage recv_msg{ackno, avai_capacity, reassembler_.writer().has_error()};
        return recv_msg;
    } else {
        TCPReceiverMessage recv_msg{std::nullopt, avai_capacity, reassembler_.writer().has_error()};
        return recv_msg;
    }
}
