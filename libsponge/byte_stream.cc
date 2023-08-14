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
    : _capacity(capacity), _error(false), _end_input(false), _write_count(0), _read_count(0), _buf({}) {
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
    _buf.push_back(std::move(str));
    _write_count += len;
    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    //    DUMMY_CODE(len);
    if (len == 0) {
        return "";
    }
    size_t pop_len = buffer_size() > len ? len : buffer_size();
    if (_buf.front().size() > pop_len) {
        return _buf.front().substr(0, pop_len);
    } else if (_buf.front().size() == pop_len) {
        return _buf.front();
    } else {
        string str;
        auto it = _buf.begin();
        //        while (it->size() <= pop_len) {
        while (it != _buf.end() && it->size() <= pop_len) {
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
    _read_count += pop_len;
    if (_buf.front().size() > pop_len) {
        _buf.front().erase(0, pop_len);
    } else if (_buf.front().size() == pop_len) {
        _buf.pop_front();
    } else {
        // 判空
        while (!_buf.empty() && _buf.front().size() <= pop_len) {
            pop_len -= _buf.front().size();
            _buf.pop_front();
        }
        if (pop_len > 0) {
            _buf.front().erase(0, pop_len);
        }
    }
}

void ByteStream::end_input() { _end_input = true; }

bool ByteStream::input_ended() const { return _end_input; }

size_t ByteStream::buffer_size() const {
    size_t _buf_bytes_size = 0;
    for (const auto &buf : _buf) {
        _buf_bytes_size += buf.size();
    }
    return _buf_bytes_size;
}

bool ByteStream::buffer_empty() const { return _buf.empty(); }

bool ByteStream::eof() const { return _end_input && _buf.empty(); }

size_t ByteStream::bytes_written() const { return _write_count; }

size_t ByteStream::bytes_read() const { return _read_count; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer_size(); }
