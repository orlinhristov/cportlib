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

#include <detail/completion_port_impl.hpp>
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

inline bool completion_port_impl::pull_one()
{
    std::unique_lock<std::mutex> lock(guard_);
    return do_one(lock);
}

inline void completion_port_impl::reset()
{
    std::unique_lock<std::mutex> lock(guard_);
    stopped_ = false;
}

inline void completion_port_impl::stop()
{
    std::unique_lock<std::mutex> lock(guard_);
    stopped_ = true;
    cond_.notify_all();
}

inline bool completion_port_impl::stopped() const
{
    std::unique_lock<std::mutex> lock(guard_);
    return stopped_;
}

inline std::size_t completion_port_impl::ready_handlers() const
{
    std::unique_lock<std::mutex> lock(guard_);
    return handlers_.size();
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

} // namespace detail

} // namespace mt

#endif // __COMPLETION_PORT_IMPL_IPP__