#ifndef __BASIC_TIMER_SERVICE_HPP__
#define __BASIC_TIMER_SERVICE_HPP__

#include <cport/timer/detail/basic_timer_service_impl.hpp>

/*
TODO:
 * Consider using util::event instead of directly using codition_variable to avoid
  the race condition at thread start.

 * Make sure heap related std functions are properly used
 
 */

namespace cport {

class completion_port;

namespace timer {

// The ClockType must meet the following requirements:
//  Declare a std::chrono::time_point type to represents a point in time
//  Declare a std::chrono::time_unit used to measure the time since epoch
//  Declare a static method now() that will return current time as a std::chrono::time_point

template <typename ClockType>
class basic_timer_service {
public:
    using impl_type = detail::basic_timer_service_impl<ClockType>;

    using clock = ClockType;

    using time_point = typename impl_type::time_point;

    using time_unit = typename impl_type::time_unit;

    using timer_callback = typename impl_type::timer_callback;

    explicit basic_timer_service(completion_port& port);

    timer_id add_timer(time_unit interval, timer_callback callback);

    void remove_timer(timer_id id, bool notify);
private:
    std::shared_ptr<impl_type> impl_;
};

} // namespace timer

} // namespace cport

#include <cport/timer/impl/basic_timer_service.inl>

#endif //__BASIC_TIMER_SERVICE_HPP__
