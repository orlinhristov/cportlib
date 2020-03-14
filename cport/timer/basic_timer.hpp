#ifndef __BASIC_TIMER_HPP__
#define __BASIC_TIMER_HPP__

#include <cport/timer/detail/basic_timer_impl.hpp>

namespace cport {

namespace timer {

/// A class used to start a timer with regular notifications on intervals

template <typename TimerService>
class basic_timer {
    /// The implementation type
    using impl_type = detail::basic_timer_impl<TimerService>;
public:
    /// The timer service type
    using basic_timer_service = typename impl_type::basic_timer_service;

    /// The time units type
    using time_unit = typename impl_type::time_unit;

    /// The time point type
    using time_point = typename impl_type::time_point;

    /// The clock type 
    using clock = typename impl_type::clock;

    /// The type of the interval callback
    using timer_callback = typename impl_type::timer_callback;

    /// Construct a coundown timer
    explicit basic_timer(basic_timer_service& service);

    /// Start the timer
    /**
     * Start a timer that will invoke the callback on regular intervals,
     *  until the timer is canceled.
     *
     * @param callback A callback function to be invoked on a regilar interval.
     *
     * @param interval The interval on which to receive regular notifications.
     *
     * @returns This method will return false if the timer is already started,
     *  otherwise it will return true.
     */
    bool start(timer_callback callback, time_unit interval);

    /// Start the timer
    /**
     * Start a timer that will invoke the callback on regular intervals
     *  and expires after a specified duration.
     *
     * @param callback A callback function to be invoked on a regilar interval.
     *
     * @param interval The interval on which to receive regular notifications.
     *
     * @returns This method will return false if the timer is already started,
     *  otherwise it will return true.
     */
    bool start(timer_callback callback, time_unit interval, time_unit expire_after);

    /// Start the timer
    /**
     * Start a timer that will invoke the callback on regular intervals
     *  and expires at specified time.
     *
     * @param callback A callback function to be invoked on a regilar interval.
     *
     * @param interval The interval on which to receive regular notifications.
     *
     * @returns This method will return false if the timer is already started,
     *  otherwise it will return true.
     */
    bool start(timer_callback callback, time_unit interval, time_point expire_time);

    /// Test if the timer is in progress
    bool started() const;

    /// Get the identifier of the timer
    /**
     * @returns 0 if the timer is not started, otherwise will return
     *  a unique identifier assigned at timer start
     */
    timer_id get_id() const;

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

#include <cport/timer/impl/basic_timer.inl>

#endif //__BASIC_TIMER_HPP__
