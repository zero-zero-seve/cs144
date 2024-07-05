#include <algorithm>
#include <cstdint>
#include <stdexcept>

#include "byte_stream.hh"

/*
 * read: A helper function thats peeks and pops up to `len` bytes
 * from a ByteStream Reader into a string;
 */
void read(Reader& reader, uint64_t len, std::string& out) {
    out.clear();
    uint64_t temp = 0;
    while (reader.bytes_buffered() and out.size() < len) {
        auto view = reader.peek();

        if (view.empty()) {
            throw std::runtime_error("Reader::peek() returned empty string_view");
        }
        temp = std::min(view.size(), len - out.size());
        view = view.substr(0, temp);  // Don't return more bytes than desired.
        out += view;
        reader.pop(view.size());
    }
}

Reader& ByteStream::reader() {
    static_assert(sizeof(Reader) == sizeof(ByteStream),
                  "Please add member variables to the ByteStream base, not the ByteStream Reader.");

    return static_cast<Reader&>(*this);  // NOLINT(*-downcast)
}

const Reader& ByteStream::reader() const {
    static_assert(sizeof(Reader) == sizeof(ByteStream),
                  "Please add member variables to the ByteStream base, not the ByteStream Reader.");

    return static_cast<const Reader&>(*this);  // NOLINT(*-downcast)
}

Writer& ByteStream::writer() {
    static_assert(sizeof(Writer) == sizeof(ByteStream),
                  "Please add member variables to the ByteStream base, not the ByteStream Writer.");

    return static_cast<Writer&>(*this);  // NOLINT(*-downcast)
}

const Writer& ByteStream::writer() const {
    static_assert(sizeof(Writer) == sizeof(ByteStream),
                  "Please add member variables to the ByteStream base, not the ByteStream Writer.");

    return static_cast<const Writer&>(*this);  // NOLINT(*-downcast)
}
