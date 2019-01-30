#ifndef __COUNTDOWN_TIMER_IMPL_INL__
#define __COUNTDOWN_TIMER_IMPL_INL__

#include <cport/timer/detail/countdown_timer_impl.hpp>

namespace cport {

namespace timer {

namespace detail {

template <typename TimerService>
countdown_timer_impl<TimerService>::countdown_timer_impl(timer_service& service)
    : service_{ service }
{    
}

template <typename TimerService>
countdown_timer_impl<TimerService>::~countdown_timer_impl()
{
    //The destructor should not be called before the timer
    // is canceled and the callback invoked.
    std::unique_lock<std::mutex> lock(guard_);

    assert(!timer_started_);
}

template <typename TimerService>
bool countdown_timer_impl<TimerService>::start(
    finish_callback finish_cb,
    tick_callback tick_cb,
    time_unit duration,
    time_unit interval)
{
    std::unique_lock<std::mutex> lock(guard_);

    if (timer_started_)
        return false;

    timer_started_ = true;

    lock.unlock();

    finish_cb_ = finish_cb;

    tick_cb_ = tick_cb;

    end_point_ = clock::now() + duration;

    timer_id_ = service_.add_timer(interval,
        std::bind(&countdown_timer_impl<TimerService>::timer_callback,
            this->shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));

    return true;
}

template <typename TimerService>
bool countdown_timer_impl<TimerService>::started() const
{
    std::unique_lock<std::mutex> lock(guard_);

    return timer_started_;
}

template <typename TimerService>
timer_id countdown_timer_impl<TimerService>::get_id() const
{
    return timer_id_;
}

template <typename TimerService>
void countdown_timer_impl<TimerService>::cancel()
{
    std::unique_lock<std::mutex> lock(guard_);

    if (timer_started_)
    {
        lock.unlock();

        service_.remove_timer(timer_id_, true);
    }
}

template <typename TimerService>
void countdown_timer_impl<TimerService>::timer_callback(
    const generic_error& ge,
    const timer_id& tid,
    const time_point& tp)
{
    if (!ge)
    {
        auto finished = tp >= end_point_;

        if (!finished)
        {
            // Invoke the tick callback passing remaining time until the deadline.
            tick_cb_(tid, std::chrono::duration_cast<time_unit>(end_point_ - tp));

            // Check again if the deadline has passed
            finished = clock::now() >= end_point_;
        }
        
        if (finished)
        {
            service_.remove_timer(tid, false);

            invoke_finish(ge, tid);
        }
    }
    else
    {
        invoke_finish(ge, tid);
    }
}

template <typename TimerService>
void countdown_timer_impl<TimerService>::invoke_finish(
    const generic_error& ge,
    const timer_id& tid
)
{
    std::unique_lock<std::mutex> lock(guard_);

    timer_started_ = false;

    lock.unlock();

    finish_cb_(ge, tid);
}

} // namespace detail

} // namespace timer

} // namespace cport

#endif //__COUNTDOWN_TIMER_IMPL_INL__