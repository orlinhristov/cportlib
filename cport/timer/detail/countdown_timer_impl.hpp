#ifndef __COUNTDOWN_TIMER_IMPL_HPP__
#define __COUNTDOWN_TIMER_IMPL_HPP__

#include <functional>
#include <mutex>

namespace cport {

namespace timer {

namespace detail {

template <typename TimerService>
class countdown_timer_impl 
    : public std::enable_shared_from_this<countdown_timer_impl<TimerService>> {
public:
	using timer_service = TimerService;

	using time_unit = typename TimerService::time_unit;

	using time_point = typename TimerService::time_point;

	using clock = typename TimerService::clock;

	using finish_callback = std::function<
        void(const generic_error&, const timer_id&)
    >;

	using tick_callback = std::function<
        void(const timer_id&, const time_unit&)
    >;

    explicit countdown_timer_impl(timer_service& service);

	countdown_timer_impl(const countdown_timer_impl&) = delete;

	countdown_timer_impl& operator=(countdown_timer_impl&) = delete;

	~countdown_timer_impl();

	bool start(
        finish_callback finish_cb,
        tick_callback tick_cb,
        time_unit duration,
        time_unit interval
	);

    bool started() const;

	void cancel();

private:
    void timer_callback(
        const generic_error& ge,
        const timer_id& tid,
        const time_point& tp
    );

    void invoke_finish(
        const generic_error& ge,
        const timer_id& tid
    );

    // Timer service used to initialize the object
	timer_service& service_;
    // Callback to invoke when the coundown finishes
    finish_callback finish_cb_;
    // Callback to invoke on a regilar interval
    tick_callback tick_cb_;
    // Time point when the coundown should finish
    time_point end_point_;
    // Timer unique identifier
    timer_id timer_id_;
    // Set to true while the timer is in progress
    bool timer_started_;
    // Timer guard
    mutable std::mutex guard_;
};

} // namespace detail

} // namespace timer

} // namespace cport

#include <cport/timer/detail/impl/countdown_timer_impl.inl>

#endif //__COUNTDOWN_TIMER_IMPL_HPP__