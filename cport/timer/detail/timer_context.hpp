#ifndef __TIMER_CONTEXT_HPP__
#define __TIMER_CONTEXT_HPP__

#include <cport/error_types.hpp>
#include <cport/timer/error_types.hpp>
#include <cport/timer/timer_id.hpp>

namespace cport {

namespace timer {

namespace detail {

template<typename TimePoint, typename TimeUnit, typename Callback>
struct timer_context {
    using time_point = TimePoint;

    using time_unit = TimeUnit;

    using timer_callback = Callback;

    timer_context(timer_id id, time_unit interval,
        timer_callback callback, time_point expire_time)
        : id_{ id }
        , next_point_{}
        , interval_{ interval }
        , end_point_{ expire_time }
        , callback_{ callback }
        , paused_{ true }
    {
    }

    timer_context(timer_id id, time_unit interval, timer_callback callback)
        : timer_context(id, interval, callback, time_point{})
    {
    }

    timer_id id() const
    {
        return id_;
    }

    bool valid() const
    {
        return interval_ != time_unit{} || end_point_ != time_point{} && callback_;
    }

    const time_point& next_point() const
    {
        return next_point_;
    }

    bool paused() const
    {
        return paused_;
    }

    void pause()
    {
        paused_ = true;
    }

    void invoke_callback(const generic_error& ge, const time_point& pt)
    {
        assert(valid());

        callback_(ge, id(), pt);
    }

    generic_error resume(time_point current_point)
    {
        if (!valid())
        {
            assert(false);

            return invalid_timer_error{};
        }

        assert(paused_);

        if (end_point_ != time_point{} && end_point_ <= current_point)
        {
            return timer::timer_expired_error{};
        }

        if (interval_ != time_unit{})
        {
            // Calc the next time point at which the timer has to be notified
            if (next_point_ == time_point{})
            {
                next_point_ = current_point + interval_;
            }
            else
            {
                assert(current_point > next_point_);

                const auto multiplier = int((current_point - next_point_) / interval_);

                next_point_ += interval_ * multiplier;

                if (next_point_ < current_point)
                {
                    next_point_ += interval_;
                }
            }

            if (end_point_ != time_point{})
            {
                next_point_ = std::min(next_point_, end_point_);
            }
        }
        else
        {
            // The end_point_ should be valid time point if the interval_ isn't.
            // Otherwise this object is not valid.
            assert(end_point_ != time_point{});

            next_point_ = end_point_;
        }

        assert(next_point_ >= current_point);

        paused_ = false;

        return generic_error{};
    }

    timer_id id_;

    time_point next_point_;

    time_point end_point_;

    time_unit interval_;

    timer_callback callback_;

    bool paused_;
};

} // namespace detail

} // namespace timer

} // namespace cport

#endif //__TIMER_CONTEXT_HPP__
