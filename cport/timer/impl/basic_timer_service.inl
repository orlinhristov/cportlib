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
timer_id basic_timer_service<ClockType>::add_timer(time_unit interval, timer_callback callback)
{
    return impl_->add_timer(interval, callback);
}

template <typename ClockType>
void basic_timer_service<ClockType>::remove_timer(timer_id id, bool notify)
{
    impl_->remove_timer(id, notify);
}

} // namespace timer

} // namespace cport

#endif //__TIMER_SERVICE_INL__
