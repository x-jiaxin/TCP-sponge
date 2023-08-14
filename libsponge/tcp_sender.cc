#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _retransmission_timeout(retx_timeout)
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return bytes_in_flight_; }

void TCPSender::fill_window() {
    TCPSegment msg;
    if (_fin) {
        return;
    }
    // SYN
    if (!_syn) {
        msg.header().syn = true;
        send_seg(msg);
        _syn = true;
        return;
    }
    uint16_t win_size = (receiver_window_size_ > 0 ? receiver_window_size_ : 1);
    // FIN
    if (_stream.eof() && _recv_seqno + win_size > _next_seqno) {
        msg.header().fin = true;
        send_seg(msg);
        _fin = true;
        return;
    }
    // 在接受者窗口范围内
    while (!_stream.buffer_empty() && _recv_seqno + win_size > _next_seqno) {
        // 窗口剩余容量
        size_t send_size = min(TCPConfig::MAX_PAYLOAD_SIZE, size_t(win_size - (_next_seqno - _recv_seqno)));
        msg.payload() = _stream.read(min(send_size, _stream.buffer_size()));
        if (_stream.eof() && msg.length_in_sequence_space() < win_size) {
            msg.header().fin = true;
            _fin = true;
        }
        send_seg(msg);
    }
}
void TCPSender::send_seg(TCPSegment &msg) {
    msg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(msg);
    _segments_wait.push(msg);
    _next_seqno += msg.length_in_sequence_space();
    bytes_in_flight_ += msg.length_in_sequence_space();
    if (!_timer_running) {
        _timer_running = true;
        _cur_time = 0;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    auto ack_absno = unwrap(ackno, _isn, _next_seqno);
    if (ack_absno > _next_seqno) {
        return;
    }
    //    if (ack_absno >= _recv_seqno) {
    _recv_seqno = ack_absno;
    receiver_window_size_ = window_size;
    //    }
    // 删除已确认段
    bool ok = false;
    while (!_segments_wait.empty()) {
        auto msg = _segments_wait.front();
        if (ack_absno < unwrap(msg.header().seqno, _isn, _next_seqno) + msg.length_in_sequence_space()) {
            //            return;
            break;
        }

        _segments_wait.pop();
        ok = true;
        bytes_in_flight_ -= msg.length_in_sequence_space();
        _retransmission_timeout = _initial_retransmission_timeout;
        _retransmission_count = 0;
        _cur_time = 0;
    }
    if (ok) {
        fill_window();
    }
    if (!_segments_wait.empty()) {
        _timer_running = true;
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer_running) {
        return;
    }
    _cur_time += ms_since_last_tick;
    // 定时器过期
    if (_cur_time >= _retransmission_timeout && !_segments_wait.empty()) {
        _segments_out.push(_segments_wait.front());
        _cur_time = 0;
        if (receiver_window_size_ > 0 || _segments_wait.front().header().syn) {
            //        if (receiver_window_size_ > 0) {
            _retransmission_count++;
            _retransmission_timeout *= 2;
            _cur_time = 0;
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _retransmission_count; }

void TCPSender::send_empty_segment() {
    TCPSegment msg;
    msg.header().seqno = wrap(_next_seqno, _isn);
    segments_out().push(msg);
}
