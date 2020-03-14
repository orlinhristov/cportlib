#ifndef __COUNTDOWN_TIMER_INC__
#define __COUNTDOWN_TIMER_INC__

#include <cport/timer/countdown_timer.hpp>

namespace cport {

namespace timer {

template <typename TimerService>
countdown_timer<TimerService>::countdown_timer(basic_timer_service& service)
    : impl_{ new impl_type {service } }
{
}

template <typename TimerService>
bool countdown_timer<TimerService>::start(
    time_unit interval,
    tick_callback tick_cb,
    time_unit duration,
    finish_callback finish_cb)
{
    return impl_->start(interval, tick_cb, duration, finish_cb);
}

template <typename TimerService>
bool countdown_timer<TimerService>::started() const
{
    return impl_->started();
}

template <typename TimerService>
timer_id countdown_timer<TimerService>::get_id() const
{
    return impl_->get_id();
}

template <typename TimerService>
void countdown_timer<TimerService>::cancel()
{
    impl_->cancel();
}

} // namespace timer

} // namespace cport

#endif //__COUNTDOWN_TIMER_INC__
