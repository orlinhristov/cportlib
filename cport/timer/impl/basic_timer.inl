#ifndef __BASIC_TIMER_INL__
#define __BASIC_TIMER_INL__

#include <cport/timer/basic_timer.hpp>

namespace cport {

namespace timer {

template <typename TimerService>
basic_timer<typename TimerService>::basic_timer(basic_timer_service& service)
    : impl_{ new impl_type {service } }
{
}

template <typename TimerService>
bool basic_timer<TimerService>::start(
    timer_callback callback,
    time_unit interval)
{
    return impl_->start(callback, interval);
}

template <typename TimerService>
bool basic_timer<TimerService>::start(
    timer_callback callback,
    time_unit interval,
    time_unit expire_after)
{
    return impl_->start(callback, interval, expire_after);
}

template <typename TimerService>
bool basic_timer<TimerService>::start(
    timer_callback callback,
    time_unit interval,
    time_point expire_time)
{
    return impl_->start(callback, interval, expire_time);
}

template <typename TimerService>
bool basic_timer<TimerService>::started() const
{
    return impl_->started();
}

template <typename TimerService>
timer_id basic_timer<TimerService>::get_id() const
{
    return impl_->get_id();
}

template <typename TimerService>
void basic_timer<TimerService>::cancel()
{
    impl_->cancel();
}

} // namespace timer

} // namespace cport

#endif //__BASIC_TIMER_INL__
