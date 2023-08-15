#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <stdexcept>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : capacity_(capacity), error_(false), end_input_(false), write_count_(0), read_count_(0), buf_({}) {
    DUMMY_CODE(capacity);
}

size_t ByteStream::write(const string &data) {
    string str;
    size_t len = remaining_capacity() > data.size() ? data.size() : remaining_capacity();
    if (len < data.size()) {
        str = data.substr(0, len);
    } else {
        str = data;
    }
    buf_.push_back(std::move(str));
    write_count_ += len;
    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    //    DUMMY_CODE(len);
    if (len == 0) {
        return "";
    }
    size_t pop_len = buffer_size() > len ? len : buffer_size();
    if (buf_.front().size() > pop_len) {
        return buf_.front().substr(0, pop_len);
    } else if (buf_.front().size() == pop_len) {
        return buf_.front();
    } else {
        string str;
        auto it = buf_.begin();
        //        while (it->size() <= pop_len) {
        while (it != buf_.end() && it->size() <= pop_len) {
            str += *it;
            pop_len -= it->size();
            ++it;
        }
        if (pop_len > 0) {
            str += it->substr(0, pop_len);
        }
        return str;
    }
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    if (len == 0) {
        return;
    }
    size_t pop_len = buffer_size() > len ? len : buffer_size();
    read_count_ += pop_len;
    if (buf_.front().size() > pop_len) {
        buf_.front().erase(0, pop_len);
    } else if (buf_.front().size() == pop_len) {
        buf_.pop_front();
    } else {
        // 判空
        while (!buf_.empty() && buf_.front().size() <= pop_len) {
            pop_len -= buf_.front().size();
            buf_.pop_front();
        }
        if (pop_len > 0) {
            buf_.front().erase(0, pop_len);
        }
    }
}

void ByteStream::end_input() { end_input_ = true; }

bool ByteStream::input_ended() const { return end_input_; }

size_t ByteStream::buffer_size() const {
    size_t _buf_bytes_size = 0;
    for (const auto &buf : buf_) {
        _buf_bytes_size += buf.size();
    }
    return _buf_bytes_size;
}

bool ByteStream::buffer_empty() const { return buf_.empty(); }

bool ByteStream::eof() const { return end_input_ && buf_.empty(); }

size_t ByteStream::bytes_written() const { return write_count_; }

size_t ByteStream::bytes_read() const { return read_count_; }

size_t ByteStream::remaining_capacity() const { return capacity_ - buffer_size(); }
