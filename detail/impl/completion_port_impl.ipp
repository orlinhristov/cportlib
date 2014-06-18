#ifndef __COMPLETION_PORT_IMPL_IPP__
#define __COMPLETION_PORT_IMPL_IPP__

//
// completion_port_impl.ipp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cportlib/detail/completion_port_impl.hpp>
#include <cassert>
#include <limits>

namespace mt {

namespace detail {

completion_port_impl::completion_port_impl() 
: stopped_(false)
, wait_one_threads_(0)
, queued_ops_(0)
, seqno_(0)
{
}

completion_port_impl::~completion_port_impl()
{
    assert(queued_ops_ == 0);

    while (!handlers_.empty()) {
        auto_destroy h(handlers_.top());
        handlers_.pop();
        h->complete();
    }
}

bool completion_port_impl::wait_one()
{
    std::unique_lock<std::mutex> lock(guard_);
    while (!stopped_ && handlers_.empty() && queued_ops_> 0) {
        ++wait_one_threads_;
        cond_.wait(lock);
        --wait_one_threads_;
    }
    return do_one(lock);
}

bool completion_port_impl::run_one()
{
    std::unique_lock<std::mutex> lock(guard_);
    while (!stopped_ && handlers_.empty()) {
        cond_.wait(lock);
    }
    return do_one(lock);
}

std::size_t completion_port_impl::next_operation_id()
{
    std::unique_lock<std::mutex> lock(guard_);
    if (stopped_)
        return 0;

    ++queued_ops_;
    if (seqno_ == std::numeric_limits<std::size_t>::max())
        seqno_ = 0;

    return ++seqno_;
}

void completion_port_impl::post(completion_handler_base *h)
{
    std::unique_lock<std::mutex> lock(guard_);
    handlers_.push(h);

    assert(h->seqno() == 0 || queued_ops_ > 0);
    if (h->seqno() > 0)
        --queued_ops_;

    if (queued_ops_ == 0 && wait_one_threads_ > 0)
        cond_.notify_all();
    else
        cond_.notify_one();
}

bool completion_port_impl::do_one(std::unique_lock<std::mutex> &lock)
{
    assert(lock.owns_lock());
    if (handlers_.empty())
        return false;

    auto_destroy h(handlers_.top());
    handlers_.pop();
    lock.unlock();
    h->complete();
    return true;
}

completion_port_impl::priority_less_pred::priority_less_pred()
    : gap(std::numeric_limits<std::size_t>::max() / 2)
{
}

std::size_t completion_port_impl::priority_less_pred::diff(
    std::size_t s1,
    std::size_t s2)
{
    return s1 > s2 ? s1 - s2 : s2 - s1;
}

bool completion_port_impl::priority_less_pred::operator()(
    const completion_handler_base *h1,
    const completion_handler_base *h2)
{
    if (h1->seqno() == 0)
        return false;

    if (h2->seqno() == 0)
        return true;

    if (diff(h1->seqno(), h2->seqno()) > gap)
        return h1->seqno() < h2->seqno();
    return h1->seqno() > h2->seqno();
}

} // namespace detail

} // namespace mt

#endif // __COMPLETION_PORT_IMPL_IPP__