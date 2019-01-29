#ifndef __TIMER_SERVICE_HPP__
#define __TIMER_SERVICE_HPP__

#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <cport/completion_handler_wrapper.hpp>
#include <cport/error_types.hpp>
#include <cport/detail/null_handler_t.hpp>
#include <cport/timer/detail/timer_context.hpp>

/*
TODO:
 * Consider using util::event instead of directly using codition_variable to avoid
  the race condition at thread start.

 * Make sure heap related std functions are properly used
 
 */

namespace cport {

namespace timer {

// The ClockType must meet the following requirements:
//  Declare a std::chrono::time_point type to represents a point in time
//  Declare a std::chrono::time_unit used to measure the time since epoch
//  Declare a static method now() that will return current time as a std::chrono::time_point

template <typename ClockType>
class timer_service {
public:
	using clock = ClockType;

	using time_point = typename ClockType::time_point;

	using time_unit = typename ClockType::duration;

	using timer_callback = std::function<
		void(const generic_error&, const timer_id&, const time_point&)
	>;

	using timer_context = detail::timer_context<
		time_point, time_unit, timer_callback
	>;

    explicit timer_service(completion_port& port);

	timer_service(const timer_service&) = delete;

	timer_service& operator=(const timer_service&) = delete;

	timer_service(timer_service&&) = delete;

	timer_service& operator=(timer_service&&) = delete;

    ~timer_service();

    timer_id add_timer(time_unit interval, timer_callback callback);

    void remove_timer(timer_id id, bool notify);

private:
    void timer_thread_routine();

	template <typename Container>
    void add_timers(Container& c);

	template <typename Container>
    void del_timers(Container& c);

	template <typename Container>
    void check_ready_timers(Container& c);

	template <typename Container>
    void cancel_timers(Container& c);

    void cancel_timer(const timer_context& tc);

    void notify(
        const timer_callback& callback,
        const generic_error& e,
        const timer_id& tid,
        const time_point& tp
    );

	struct del_timer_info
	{
		del_timer_info(timer_id id, bool notify)
			: id(id), notify(notify)
		{
		}

		timer_id id;

		bool notify;
	};

	std::vector<del_timer_info> del_timers_;

	std::vector<timer_context> new_timers_;

	std::thread timer_thread_;

	std::condition_variable cond_var_;

	std::mutex guard_;

	completion_port& port_;

    using handler_wrapper = completion_handler_wrapper<
        cport::detail::null_handler_t
    >;
    
    handler_wrapper port_release_wrapper_;

	bool stop_thread_ = false;
};

} // namespace timer

} // namespace cport

#include <cport/timer/impl/timer_service.inl>

#endif //__TIMER_SERVICE_HPP__