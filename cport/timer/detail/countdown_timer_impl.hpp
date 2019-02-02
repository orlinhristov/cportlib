#ifndef __REGULAR_TIMER_IMPL_HPP__
#define __REGULAR_TIMER_IMPL_HPP__

#include <cport/timer/detail/basic_timer_impl.hpp>

namespace cport {

namespace timer {

namespace detail {

template <typename TimerService>
class countdown_timer_impl : public basic_timer_impl<TimerService>
    , public std::enable_shared_from_this<countdown_timer_impl<TimerService>>
{
    using base_class = basic_timer_impl<TimerService>;
public:
    using basic_timer_service = TimerService;

    using time_unit = base_class::time_unit;

    using time_point = base_class::time_point;

    using clock = base_class::clock;

    using finish_callback = std::function<
        void(const generic_error&, const timer_id&)
    >;

    using tick_callback = std::function<
        void(const timer_id&, const time_unit&)
    >;

    explicit countdown_timer_impl(basic_timer_service& service);

    bool start(
        time_unit interval,
        tick_callback tick_cb,
        time_unit duration,
        finish_callback finish_cb
    );

private:
    void timer_cb(
        const generic_error& ge,
        const timer_id& tid,
        const time_point& tp
    );

    // Callback to invoke when the coundown finishes
    finish_callback finish_cb_;
    // Callback to invoke on a regilar interval
    tick_callback tick_cb_;
    // Time point when the coundown should finish
    time_point end_point_;
    // Timer unique identifier
};

} // namespace detail

} // namespace timer

} // namespace cport

#include <cport/timer/detail/impl/countdown_timer_impl.inl>

#endif //__REGULAR_TIMER_IMPL_HPP__
