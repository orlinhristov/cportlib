#ifndef TIMER_SERVICE_HPP_INCLUDED
#define TIMER_SERVICE_HPP_INCLUDED

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

 * Think of using completion_port_wrapper to ensure completion_port::wait() like
  methods will block while there are registered timers.
 
 * Make sure heap related std functions are properly used
 
 */

namespace cport
{

namespace timer
{

// The ClockType must meet the following requirements:
//  Declare a std::chrono::time_point type to represents a point in time
//  Declare a std::chrono::time_unit used to measure the time since epoch
//  Declare a static method now() that will return current time as a std::chrono::time_point

template <typename ClockType>
class timer_service
{
public:
	using clock = ClockType;

	using time_point = typename ClockType::time_point;

	using time_unit = typename ClockType::duration;

	using timer_callback = std::function<
		void(const generic_error&,const time_point&)
	>;

	using timer_context = detail::timer_context<
		time_point, time_unit, timer_callback
	>;

    explicit timer_service(completion_port& port)
		: port_(port)
	{
		timer_thread_ = std::move(
			std::thread(
				std::bind(
					&timer_service::timer_thread_routine,
					this)));
	}

	timer_service(const timer_service&) = delete;

	timer_service& operator=(const timer_service&) = delete;

	timer_service(timer_service&&) = delete;

	timer_service& operator=(timer_service&&) = delete;

	~timer_service()
	{
		std::unique_lock<std::mutex> lock(guard_);

		stop_thread_ = true;

		cond_var_.notify_all();

		lock.unlock();

		if (timer_thread_.joinable())
		{
			timer_thread_.join();
		}
	}

	timer_id add_timer(time_unit interval, timer_callback callback)
	{
		const auto next_timer_id = get_next_timer_id();

		std::unique_lock<std::mutex> lock(guard_);

		new_timers_.emplace_back(next_timer_id, interval, callback);

        new_timers_.back().next_point = interval + clock::now();
        
        // Enforce the completion port to block on wait functions
        if (!port_release_wrapper_)
        {
            port_release_wrapper_ = std::move(
                wrap_completion_handler(
                    cport::detail::null_handler_t{},
                    port_));
        }

		cond_var_.notify_all();

		return next_timer_id;
	}

	void remove_timer(timer_id id, bool notify)
	{
		auto success = false;

		std::unique_lock<std::mutex> lock(guard_);

		del_timers_.emplace_back(id, notify);

		cond_var_.notify_all();
	}

private:
	void timer_thread_routine()
	{
		std::vector<timer_context> timer_heap;

		for (;;)
		{
			std::unique_lock<std::mutex> lock(guard_);

			if (!stop_thread_ && new_timers_.empty() && del_timers_.empty())
			{
				if (timer_heap.empty())
				{
					// Release all threads waiting for completion.
                    if (port_release_wrapper_)
                    {
                        port_release_wrapper_();
                    }
					// There is no timers to watch on, just block the thread until
					// notified for new events
					cond_var_.wait(lock);
				}
				else
				{
					// We have at least one timer to watch on! Wait until
					// the most recent time occurs or some a new event
					// is set.
					const auto& next_timer = timer_heap.front();

					cond_var_.wait_until(lock, next_timer.next_point);
				}
			}

			if (stop_thread_)
			{
				lock.unlock();

				cancel_timers(timer_heap);

				break;
			}

			del_timers(timer_heap);

			add_timers(timer_heap);

			lock.unlock();

			check_ready_timers(timer_heap);
		}		
	}

	template <typename Container>
	void add_timers(Container& c)
	{
		if (!new_timers_.empty())
		{
			for (auto& new_timer : new_timers_)
			{
				c.push_back(new_timer);
			}

			new_timers_.clear();
		}
	}

	template <typename Container>
	void del_timers(Container& c)
	{
		if (!del_timers_.empty())
		{
			const auto it = std::remove_if(c.begin(),
				c.end(), [this](const timer_context& ctxt)
			{
				for (auto del_timer : del_timers_)
				{
					if (del_timer.id == ctxt.id)
					{
						if (del_timer.notify)
						{
							cancel_timer(ctxt);
						}
						return true;
					}
				}
				return false;
			});

			c.erase(it, c.end());

			del_timers_.clear();
		}
	}

	template <typename Container>
	void check_ready_timers(Container& c)
	{
		static const auto heap_pred = [](const timer_context& a, const timer_context& b)
		{
			return !(a.next_point < b.next_point);
		};

		if (c.empty())
			return;

        const auto current_point = clock::now();

		std::make_heap(c.begin(), c.end(), heap_pred);

		std::pop_heap(c.begin(), c.end(), heap_pred);

		auto& tc = c.front();

		while (!c.empty() && current_point >= tc.next_point)
		{
			tc.next_point += tc.interval;

			notify(tc.callback, generic_error{}, current_point);

			std::pop_heap(c.begin(), c.end(), heap_pred);

			tc = c.front();
		}
	}

	template <typename Container>
	void cancel_timers(Container& c)
	{
		for (auto& tc : c)
		{
			cancel_timer(tc);
		}
	}

	void cancel_timer(const timer_context& tc)
	{
		static const operation_aborted_error abort_error{};

		static const time_point null_point{};

		notify(tc.callback, abort_error, null_point);
	}

	void notify(
		const timer_callback& callback,
		const generic_error& e,
		const time_point& tp)
	{
		port_.post([callback, tp](const generic_error& e)
		{
			callback(e, tp);
		}, e);
	}

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

	completion_handler_wrapper<cport::detail::null_handler_t> port_release_wrapper_;

	bool stop_thread_ = false;
};

} // namespace timer

} // namespace cport

#endif //TIMER_SERVICE_HPP_INCLUDED