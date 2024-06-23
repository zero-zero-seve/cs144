#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream(uint64_t capacity) : capacity_(capacity), buffer(capacity, '\0') { buffer.clear(); }
bool Writer::is_closed() const { return this->closed; }
void Writer::push(string data) {
    string temp = data;
    if (available_capacity() == 0) {
        return;
    }
    if (data.size() > this->available_capacity()) {
        // ¼õÉÙdata
        temp = data.substr(0, this->available_capacity());
    }
    buffer = buffer + temp;
    count_w += temp.size();
    return;
}

void Writer::close() {
    // Your code here.
    this->closed = true;
}

uint64_t Writer::available_capacity() const {
    // Your code here.
    return this->capacity_ - this->buffer.size();
}

uint64_t Writer::bytes_pushed() const { return count_w; }

bool Reader::is_finished() const {
    // Your code here.
    return this->closed && buffer.size() == 0;
}

uint64_t Reader::bytes_popped() const {
    // Your code here.
    return count_r;
}

string_view Reader::peek() const { return string_view(buffer); }

void Reader::pop(uint64_t len) {
    if (buffer.size() <= len) {
        count_r += buffer.size();
        buffer.clear();
        return;
    } else {
        count_r += len;
        string str(buffer);
        str = str.substr(len);
        buffer.clear();
        buffer += str;
    }
}

uint64_t Reader::bytes_buffered() const { return buffer.size(); }
