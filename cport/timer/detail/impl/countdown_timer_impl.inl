#ifndef __COUNTDOWN_TIMER_IMPL_INL__
#define __COUNTDOWN_TIMER_IMPL_INL__

#include <cport/timer/detail/countdown_timer_impl.hpp>

namespace cport {

namespace timer {

namespace detail {

template <typename TimerService>
countdown_timer_impl<TimerService>::countdown_timer_impl(basic_timer_service& service)
    : base_class{ service }
{    
}

template <typename TimerService>
bool countdown_timer_impl<TimerService>::start(
    time_unit interval,
    tick_callback tick_cb,
    time_unit duration,
    finish_callback finish_cb)
{
    const auto expire_time = clock::now() + duration;

    return base_class::start(interval,
        std::bind(&countdown_timer_impl<TimerService>::timer_cb,
            this->shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3),
            expire_time,
            [=]() {
                finish_cb_ = finish_cb;

                tick_cb_ = tick_cb;

                end_point_ = expire_time;
            });
}

template <typename TimerService>
void countdown_timer_impl<TimerService>::timer_cb(
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
            base_class::service().remove_timer(tid, false);

            finish_cb_(ge, tid);
        }
    }
    else
    {
        finish_cb_(ge, tid);
    }
}

} // namespace detail

} // namespace timer

} // namespace cport

#endif //__COUNTDOWN_TIMER_IMPL_INL__
