#ifndef __TIMER_SERVICE_INL__
#define __TIMER_SERVICE_INL__

#include <cport/timer/basic_timer_service.hpp>

namespace cport {

namespace timer {

template <typename ClockType>
basic_timer_service<ClockType>::basic_timer_service(completion_port& port)
    : impl_{ new impl_type{port} }
{
}

template <typename ClockType>
timer_id basic_timer_service<ClockType>::add_timer(timer_callback callback, time_unit interval)
{
    return impl_->add_timer(callback, interval);
}

template <typename ClockType>
timer_id basic_timer_service<ClockType>::add_timer(timer_callback callback, time_unit interval, time_unit expire_after)
{
    return impl_->add_timer(callback, interval, expire_after);
}

template <typename ClockType>
timer_id basic_timer_service<ClockType>::add_timer(timer_callback callback, time_unit interval, time_point expire_time)
{
    return impl_->add_timer(callback, interval, expire_time);
}

template <typename ClockType>
void basic_timer_service<ClockType>::remove_timer(timer_id id, bool notify)
{
    impl_->remove_timer(id, notify);
}

} // namespace timer

} // namespace cport

#endif //__TIMER_SERVICE_INL__
