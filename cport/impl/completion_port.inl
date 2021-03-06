#ifndef __COMPLETION_PORT_INL__
#define __COMPLETION_PORT_INL__

//
// completion_port.inl
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/error_types.hpp>

namespace cport {

template <typename Handler>
inline void completion_port::dispatch(Handler&& h, const generic_error& e)
{
    impl().dispatch(std::forward<Handler>(h), e);
}

template <typename Handler>
inline void completion_port::dispatch(Handler&& h)
{
    dispatch(h, generic_error());
}

template <typename Handler>
inline void completion_port::post(Handler&& h, const generic_error& e)
{
    impl().post(std::forward<Handler>(h), impl_.next_operation_id(), e);
}

template <typename Handler>
inline void completion_port::post(Handler&& h)
{
    post(std::forward<Handler>(h), generic_error());
}

template <typename Handler>
inline void completion_port::call(Handler&& h, const generic_error& e)
{
    impl().call(std::forward<Handler>(h), e);
}

template <typename Handler>
inline void completion_port::call(Handler&& h)
{
    impl().call(std::forward<Handler>(h), generic_error());
}

inline std::size_t completion_port::wait()
{
    std::size_t count = 0;
    while (wait_one()) {
        if (count < std::numeric_limits<std::size_t>::max()) {
            ++count;
        }
    }
    return count;
}

inline bool completion_port::wait_one()
{
    return impl().wait_one();
}

inline std::size_t completion_port::run()
{
    std::size_t count = 0;
    while (run_one()) {
        if (count < std::numeric_limits<std::size_t>::max()) {
            ++count;
        }
    }
    return count;
}

inline bool completion_port::run_one()
{
    return impl().run_one();
}

inline std::size_t completion_port::pull()
{
    std::size_t count = 0;
    while (pull_one()) {
        if (count < std::numeric_limits<std::size_t>::max()) {
            ++count;
        }
    }
    return count;
}

inline bool completion_port::pull_one()
{
    return impl().pull_one();
}

inline void completion_port::stop()
{
    impl().stop();
}

inline bool completion_port::stopped() const
{
    return impl().stopped();
}

inline void completion_port::reset()
{
    impl().reset();
}

inline std::size_t completion_port::ready_handlers() const
{
    return impl().ready_handlers();
}

inline std::size_t completion_port::blocked_threads() const
{
    return impl().blocked_threads();
}

inline const completion_port::impl_type& completion_port::impl() const
{
    return impl_;
}

inline completion_port::impl_type& completion_port::impl()
{
    return impl_;
}

} // namespace cport

#endif//__COMPLETION_PORT_INL__
