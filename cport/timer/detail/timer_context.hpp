#ifndef __TIMER_CONTEXT_HPP__
#define __TIMER_CONTEXT_HPP__

#include <cport/error_types.hpp>
#include <cport/timer/timer_id.hpp>

namespace cport {

namespace timer {

namespace detail {

template<typename TimePoint, typename TimeUnit, typename Callback>
struct timer_context {
    using time_point = TimePoint;

    using time_unit = TimeUnit;

    timer_context(timer_id id, TimeUnit interval, Callback callback)
        : id(id), next_point(), interval(interval), callback(callback)
    {
    }

    timer_context(const timer_context&) = default;

    timer_context& operator=(const timer_context&) = default;

    timer_context(timer_context&&) = default;

    timer_context& operator=(timer_context&&) = default;

    timer_id id;

    time_point  next_point;

    time_unit   interval;

    Callback   callback;

    bool paused = false;
};

} // namespace detail

} // namespace timer

} // namespace cport

#endif //__TIMER_CONTEXT_HPP__