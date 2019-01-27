#ifndef COUNTDOWN_TIMER_HPP_INCLUDED
#define COUNTDOWN_TIMER_HPP_INCLUDED

#include <functional>

/*
TODO:
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

	using finish_callback = std::function<
        void(const generic_error&, const timer_id&)
    >;

	using tick_callback = std::function<
        void(const timer_id&, const time_unit&)
    >;

	countdown_timer(
		timer_service& service,
		finish_callback finish_cb,
		tick_callback  tick_cb
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
    struct timer_data
    {
        finish_callback finish_cb;
	    tick_callback   tick_cb;
	    time_point      end_point;
    };

	timer_service& service_;
    timer_id       timer_id_;
    timer_data     data_;
};

template <typename ClockType>
countdown_timer<typename ClockType>::countdown_timer(
	timer_service& service,
	finish_callback finish_cb,
	tick_callback tick_cb)
    : service_{ service }
    , timer_id_{ }
    , data_{ finish_cb, tick_cb, {} }
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
	data_.end_point = clock::now() + duration;

    timer_id_ = service_.add_timer(interval, [&service = service_, data = data_]
    (const generic_error& ge, const timer_id& tid, const time_point& tp)
	{
		if (!ge && tp < data.end_point)
		{
			using namespace std::chrono;

			data.tick_cb(tid, duration_cast<time_unit>(data.end_point - tp));
		}
		else
		{
			service.remove_timer(tid, false);

			data.finish_cb(ge, tid);
		}
	});

	return *this;
}

template <typename ClockType>
countdown_timer<typename ClockType>& countdown_timer<typename ClockType>::cancel()
{
    static const auto invalid_timer_id = timer_id{};

	if (timer_id_ != invalid_timer_id)
	{
		service_.remove_timer(timer_id_, true);

		timer_id_ = invalid_timer_id;
	}

	return *this;
}

} // namespace timer

} // namespace cport

#endif //COUNTDOWN_TIMER_HPP_INCLUDED