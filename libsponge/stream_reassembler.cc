#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t first_index, bool is_last_substring) {
    DUMMY_CODE(data, first_index, is_last_substring);
    if (data.empty()) {
        // "" with eof
        if (is_last_substring) {
            _output.end_input();
        }
        return;
    }
    if (_output.remaining_capacity() == 0) {
        return;
    }
    string s = data;
    const auto end_next_index = first_index + s.size();
    const auto first_unaccept_index = _output.remaining_capacity() + _first_unassembled_index;

    if (end_next_index <= _first_unassembled_index || first_index >= first_unaccept_index) {
        return;
    }
    // end_next_index > _first_unassembled_index && first_index < first_unaccept_index
    if (end_next_index > first_unaccept_index) {
        s = s.substr(0, first_unaccept_index - first_index);
        _is_last_substr = false;
        is_last_substring = false;
    }
    // end_next_index < first_unaccept_index

    if (first_index > _first_unassembled_index) {
        insert_buf(first_index, std::move(s), is_last_substring);
        return;
    }
    if (first_index < _first_unassembled_index) {
        s = s.substr(_first_unassembled_index - first_index);
    }
    // first_index = _first_unassembled_index
    _first_unassembled_index += s.size();
    _output.write(s);
    if (is_last_substring) {
        _output.end_input();
    }
    if (!_reassembler_buf.empty() && _reassembler_buf.begin()->first <= _first_unassembled_index) {
        pop_buf();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _reassembler_bufsize; }

bool StreamReassembler::empty() const { return _reassembler_buf.empty(); }
void StreamReassembler::insert_buf(uint64_t first_index, string &&data, bool is_last) {
    // first_index > _first_unassembled_index
    auto begin_index = first_index;
    auto end_next_index = first_index + data.size();
    for (auto it = _reassembler_buf.begin(); it != _reassembler_buf.end() && begin_index < end_next_index;) {
        if (it->first <= begin_index) {
            begin_index = max(begin_index, it->first + it->second.size());
            ++it;
            continue;
        }
        //    else if ( it->first == begin_index ) {
        //      if ( it->second.size() < data.size() ) {
        //        it->second = data;
        //      }
        //    }

        // full insert
        if (begin_index == first_index && end_next_index <= it->first) {
            _reassembler_bufsize += data.size();
            // insert before it
            _reassembler_buf.emplace(it, begin_index, std::move(data));
            return;
        }
        // end_next_index > it->first
        const auto right_next_index = min(it->first, end_next_index);
        const auto len = right_next_index - begin_index;
        _reassembler_buf.emplace(it, begin_index, data.substr(begin_index - first_index, len));
        _reassembler_bufsize += len;
        begin_index = right_next_index;
    }
    if (end_next_index > begin_index) {
        _reassembler_bufsize += end_next_index - begin_index;
        _reassembler_buf.emplace_back(begin_index, data.substr(begin_index - first_index));
    }
    if (is_last) {
        _is_last_substr = true;
    }
}
void StreamReassembler::pop_buf() {
    //_reassembler_buf.begin()->first <= _first_unassembled_index
    for (auto it = _reassembler_buf.begin(); it != _reassembler_buf.end();) {
        if (it->first > _first_unassembled_index) {
            break;
        }
        // it->first <= _first_unassembled_index
        const auto end = it->first + it->second.size();
        if (end <= _first_unassembled_index) {
            _reassembler_bufsize -= it->second.size();
        } else {
            auto data = std::move(it->second);
            _reassembler_bufsize -= data.size();
            if (it->first < _first_unassembled_index) {
                data = data.substr(_first_unassembled_index - it->first);
            }
            _first_unassembled_index += data.size();
            _output.write(data);
        }
        //    最后移除元素之后的迭代器。
        it = _reassembler_buf.erase(it);
    }
    if (_reassembler_buf.empty() && _is_last_substr) {
        _output.end_input();
    }
}
