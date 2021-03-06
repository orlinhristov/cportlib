#ifndef __COMPLETION_PORT_IMPL_INL__
#define __COMPLETION_PORT_IMPL_INL__

//
// completion_port_impl.inl
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/detail/completion_handler.hpp>
#include <cassert>

namespace cport {

namespace detail {

template <typename Handler>
inline void completion_port_impl::post(Handler&& h, std::size_t seqno, const generic_error& e)
{
    post(create_completion_handler(std::forward<Handler>(h), seqno, e));
}

template <typename Handler>
inline void completion_port_impl::dispatch(Handler&& h, const generic_error& e)
{
    post(std::forward<Handler>(h), 0, e);
}

template <typename Handler>
inline void completion_port_impl::call(Handler&& h, const generic_error& e)
{
    h(e);
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

inline std::size_t completion_port_impl::blocked_threads() const
{
    std::unique_lock<std::mutex> lock(guard_);
    return wait_one_threads_ + run_one_threads_;
}

} // namespace detail

} // namespace cport

#endif // __COMPLETION_PORT_IMPL_INL__
