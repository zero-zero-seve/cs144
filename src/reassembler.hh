#pragma once

#include <deque>
#include <map>
#include <string>
#include <utility>

#include "byte_stream.hh"
class Reassembler {
   public:
    // Construct Reassembler to write into given ByteStream.
    explicit Reassembler(ByteStream&& output) : output_(std::move(output)) {}

    void insert(uint64_t first_index, std::string data, bool is_last_substring);

    // How many bytes are stored in the Reassembler itself?
    uint64_t bytes_pending() const;

    // Access output stream reader
    Reader& reader() { return output_.reader(); }
    const Reader& reader() const { return output_.reader(); }

    // Access output stream writer, but const-only (can't write from outside)
    const Writer& writer() const { return output_.writer(); }

   private:
    std::deque<std::pair<uint64_t, std::string>> byteQueue{};
    ByteStream output_;     // the Reassembler writes to this ByteStream
    bool isGetLast{false};  // 期待收到的下一个字节在字节流中位置
    uint64_t doc{0};
};
