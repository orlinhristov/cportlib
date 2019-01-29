#ifndef __COUNTDOWN_TIMER_HPP__
#define __COUNTDOWN_TIMER_HPP__

#include <cport/timer/detail/countdown_timer_impl.hpp>

namespace cport {

namespace timer {

/// A class used to schedule a countdown timer with a specific duration
///  and regular notifications on intervals

template <typename TimerService>
class countdown_timer {
    /// The implementation type
    using impl_type = detail::countdown_timer_impl<TimerService>;
public:
    /// The timer service type
    using timer_service = typename impl_type::timer_service;

    /// The time units type
	using time_unit = typename impl_type::time_unit;

    /// The time point type
	using time_point = typename impl_type::time_point;

    /// The clock type 
	using clock = typename impl_type::clock;

    /// The type of the finish callback
    using finish_callback = typename impl_type::finish_callback;

    /// The type of the interval callback
	using tick_callback = typename impl_type::tick_callback;

    /// Construct a coundown timer
    explicit countdown_timer(timer_service& service);

    /// Disable copy constructor.
	countdown_timer(const countdown_timer&) = delete;

    /// Disable assignment operator.
	countdown_timer& operator=(countdown_timer&) = delete;

    /// Destruct the timer
    /**
     * Cancel the timer if it is started and not finished.
     */
	~countdown_timer();

    /// Start the countdown timer
    /**
     * Start a countdown that will invoke a callbacks
     *  on regular interval and on finish.
     *
     * @param finish_cb A callback to be invoked when the countdown finishes.
     * 
     * @param tick_cb A callback to be invoked on a regilar interval.
     *
     * @param duration The duration after which the coundown should finish.
     *
     * @param interval The interval on which to receive regular notifications
     *
     * @returns This method will return false if the timer is already started,
     *  otherwise it will return true.
     */
	bool start(
        finish_callback finish_cb,
        tick_callback tick_cb,
        time_unit duration,
        time_unit interval
	);

    /// Test if the timer is in progress
    bool started() const;

    /// Cancel the timer
    /**
     * This method has no effect if the timer was not start
     *  or was finished
     */
	void cancel();

private:
    std::shared_ptr<impl_type> impl_;
};

} // namespace timer

} // namespace cport

#include <cport/timer/impl/countdown_timer.inl>

#endif //__COUNTDOWN_TIMER_HPP__