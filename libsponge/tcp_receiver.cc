#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    DUMMY_CODE(seg);
    if (!seg.header().syn && !_isn.has_value()) {
        return;
    }
    if (seg.header().syn) {
        _isn = seg.header().seqno;
    }
    auto check = _reassembler.stream_out().bytes_written() + 1;
    auto abs_seqno = unwrap(seg.header().seqno, _isn.value(), check);
    auto index = seg.header().syn ? 0 : abs_seqno - 1;
    _reassembler.push_substring(seg.payload().copy(), index, seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_isn.has_value()) {
        return nullopt;
    }
    uint64_t const abs_seqno = _reassembler.stream_out().bytes_written() + 1 + _reassembler.stream_out().input_ended();
    // 序列号
    return wrap(abs_seqno, _isn.value());
}

size_t TCPReceiver::window_size() const {
    auto write_available_size = _reassembler.stream_out().remaining_capacity();
    //    if (write_available_size >= UINT16_MAX) {
    //        write_available_size = UINT16_MAX;
    //    }
    return write_available_size;
}
