#ifndef __BASIC_TIMER_IMPL_INL__
#define __BASIC_TIMER_IMPL_INL__

#include <cport/timer/detail/basic_timer_impl.hpp>

namespace cport {

namespace timer {

namespace detail {

template <typename TimerService>
basic_timer_impl<TimerService>::basic_timer_impl(basic_timer_service& service)
    : service_{ service }, timer_id_{}, timer_started_(false)
{
}

template <typename TimerService>
basic_timer_impl<TimerService>::~basic_timer_impl()
{
    cancel();
}

template <typename TimerService>
bool basic_timer_impl<TimerService>::start(
    time_unit interval,
    timer_callback callback)
{
    static auto null_initalizer = []() {};

    return start(interval, callback, null_initalizer);
}

template <typename TimerService>
bool basic_timer_impl<TimerService>::started() const
{
    std::unique_lock<std::mutex> lock(guard_);

    return timer_started_;
}

template <typename TimerService>
TimerService& basic_timer_impl<TimerService>::service() const
{
    return service_;
}

template <typename TimerService>
timer_id basic_timer_impl<TimerService>::get_id() const
{
    return timer_id_;
}

template <typename TimerService>
void basic_timer_impl<TimerService>::cancel()
{
    std::unique_lock<std::mutex> lock(guard_);

    if (timer_started_)
    {
        timer_started_ = false;

        const auto id = timer_id_;

        lock.unlock();

        service_.remove_timer(id, true);
    }
}

template <typename TimerService>
template <typename Func>
bool basic_timer_impl<TimerService>::start(
    time_unit interval,
    timer_callback callback,
    Func initalizer)
{
    assert(callback);

    std::unique_lock<std::mutex> lock(guard_);

    if (timer_started_)
        return false;

    initalizer();

    timer_started_ = true;

    timer_id_ = service_.add_timer(interval, callback);

    return true;
}

} // namespace detail

} // namespace timer

} // namespace cport

#endif //__BASIC_TIMER_IMPL_INL__
