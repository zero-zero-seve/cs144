#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class TCPSender {
   public:
    /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
    TCPSender(ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms)
        : input_(std::move(input)),
          isn_(isn),
          initial_RTO_ms_(initial_RTO_ms),
          remaining_time(initial_RTO_ms) {}

    /* Generate an empty TCPSenderMessage */
    TCPSenderMessage make_empty_message() const;

    /* Receive and process a TCPReceiverMessage from the peer's receiver */
    void receive(const TCPReceiverMessage& msg);

    /* Type of the `transmit` function that the push and tick methods can use to send messages */
    using TransmitFunction = std::function<void(const TCPSenderMessage&)>;

    /* Push bytes from the outbound stream */
    void push(const TransmitFunction& transmit);

    /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
    void tick(uint64_t ms_since_last_tick, const TransmitFunction& transmit);

    // Accessors
    uint64_t sequence_numbers_in_flight() const;   // How many sequence numbers are outstanding?
    uint64_t consecutive_retransmissions() const;  // How many consecutive *re*transmissions have happened?
    Writer& writer() { return input_.writer(); }
    const Writer& writer() const { return input_.writer(); }

    // Access input stream reader, but const-only (can't read from outside)
    const Reader& reader() const { return input_.reader(); }

   private:
    // Variables initialized in constructor
    ByteStream input_;
    Wrap32 isn_;
    uint64_t initial_RTO_ms_;
    // TODO: 维护窗口大小使用
    uint64_t windowSize{1};
    // TODO: 最近从接受方收到的ack号，即期待我发送下一个序列号
    uint64_t recent_ackSeq{0};
    // TODO: 下一个可用序列号
    uint64_t next_usefulSeq{0};
    bool isSyn{false};
    uint64_t consec_retransmission_numbers{0};
    std::deque<std::pair<uint64_t, TCPSenderMessage>> flight_msg;
    uint64_t remaining_time;
    bool isStoped{true};
    bool isFin{false};
    uint64_t first_unacked_seq{0};
};
