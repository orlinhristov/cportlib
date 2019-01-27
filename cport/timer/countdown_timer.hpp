#ifndef COUNTDOWN_TIMER_HPP_INCLUDED
#define COUNTDOWN_TIMER_HPP_INCLUDED

#include <functional>

/*
TODO:
 * Think of moving all the data members is copyable structure to avoid race
  condition when the timer object is destroyed before the callback invocation

 * The timer cancelation should call the callback, e.g. call timer_service::remove_timer
    with notify

 * Change the finish callback to accept generic_erro instead of bool flag. 

 * Think of how to round the remainig time (current_point += floor(mod(current_point, interval) + interval/2))
*/

namespace cport
{

namespace timer
{

template <typename TimerService>
class countdown_timer
{
public:
	using timer_service = TimerService;

	using time_unit = typename TimerService::time_unit;

	using time_point = typename TimerService::time_point;

	using clock = typename TimerService::clock;

	using finish_callback = std::function<void(generic_error)>;

	using tick_callback = std::function<void(time_unit)>;

	countdown_timer(
		timer_service& service,
		finish_callback fcb,
		tick_callback  tcb
	);

	countdown_timer(const countdown_timer&) = delete;

	countdown_timer& operator=(countdown_timer&) = delete;

	~countdown_timer();

	countdown_timer& start(
		time_unit duration,
		time_unit interval
	);

	countdown_timer& cancel();

private:
	timer_id timer_id_;
	timer_service& service_;
	finish_callback finish_callback_;
	tick_callback tick_callback_;
	time_unit duration_;
	time_unit interval_;
	time_point end_point_;
};

template <typename ClockType>
countdown_timer<typename ClockType>::countdown_timer(
	timer_service& service,
	finish_callback finish_callback,
	tick_callback tick_callback)
	: service_(service)
	, finish_callback_(finish_callback)
	, tick_callback_(tick_callback)
{
}

template <typename ClockType>
countdown_timer<typename ClockType>::~countdown_timer()
{
	cancel();
}

template <typename ClockType>
countdown_timer<typename ClockType>& countdown_timer<typename ClockType>::start(
	time_unit duration,
	time_unit interval)
{
	end_point_ = clock::now() + duration;

	duration_ = duration;

	interval_ = interval;

	timer_id_ = service_.add_timer(interval,
		[this](const generic_error& e, const time_point& tp)
	{
		if (!e && tp < end_point_)
		{
			using namespace std::chrono;

			tick_callback_(duration_cast<time_unit>(end_point_ - tp));
		}
		else
		{
			service_.remove_timer(timer_id_, false);

			finish_callback_(e);
		}
	});

	return *this;
}

template <typename ClockType>
countdown_timer<typename ClockType>& countdown_timer<typename ClockType>::cancel()
{
	if (timer_id_ != timer_id{})
	{
		service_.remove_timer(timer_id_, true);

		timer_id_ = timer_id{};
	}

	return *this;
}

} // namespace timer

} // namespace cport

#endif //COUNTDOWN_TIMER_HPP_INCLUDED