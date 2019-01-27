#ifndef TIMER_CONTEXT_HPP_INCLUDED
#define TIMER_CONTEXT_HPP_INCLUDED

#include <cport/error_types.hpp>
#include <cport/timer/timer_id.hpp>

namespace cport
{

namespace timer
{

namespace detail
{

template<typename TimePoint, typename TimeUnit, typename Callback>
struct timer_context
{
	using time_point = TimePoint;

	using time_unit = TimeUnit;

	timer_context(timer_id id, TimeUnit interval, Callback callback)
		: id(id), next_point(next_point), interval(interval), callback(callback)
	{
	}

	timer_context(const timer_context&) = default;

	timer_context& operator=(const timer_context&) = default;

	timer_context(timer_context&&) = default;

	timer_context& operator=(timer_context&&) = default;

	timer_id id;

	time_point  next_point;

	time_unit   interval;

	Callback   callback;
};

} // namespace detail

} // namespace timer

} // namespace cport

#endif //TIMER_CONTEXT_HPP_INCLUDED