#ifndef __TIMER_SERVICE_INL__
#define __TIMER_SERVICE_INL__

#include <cport/timer/timer_service.hpp>

namespace cport {

namespace timer {

template <typename ClockType>
timer_service<ClockType>::timer_service(completion_port& port)
	: port_(port)
{
    timer_thread_ = std::move(
        std::thread(
            std::bind(
                &timer_service::timer_thread_routine,
                this)));
}

template <typename ClockType>
timer_service<ClockType>::~timer_service()
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

template <typename ClockType>
timer_id timer_service<ClockType>::add_timer(time_unit interval, timer_callback callback)
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

template <typename ClockType>
void timer_service<ClockType>::remove_timer(timer_id id, bool notify)
{
    auto success = false;

    std::unique_lock<std::mutex> lock(guard_);

    del_timers_.emplace_back(id, notify);

    cond_var_.notify_all();
}

template <typename ClockType>
void timer_service<ClockType>::timer_thread_routine()
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

        add_timers(timer_heap);

        del_timers(timer_heap);

        lock.unlock();

        check_ready_timers(timer_heap);
    }
}

template <typename ClockType>
template <typename Container>
void timer_service<ClockType>::add_timers(Container& c)
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

template <typename ClockType>
template <typename Container>
void timer_service<ClockType>::del_timers(Container& c)
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

template <typename ClockType>
template <typename Container>
void timer_service<ClockType>::check_ready_timers(Container& c)
{
    static const auto heap_pred = [](const timer_context& a, const timer_context& b)
    {
        return !(a.next_point < b.next_point);
    };

    if (c.empty())
        return;

    const auto current_point = clock::now();

    // Rearrange the timers so that the one with the 
    // most recent time will go on top
    std::make_heap(c.begin(), c.end(), heap_pred);

    std::pop_heap(c.begin(), c.end(), heap_pred);

    auto& tc = c.back();

    while (!c.empty() && current_point >= tc.next_point)
    {
        tc.next_point += tc.interval;

        notify(tc.callback, generic_error{}, tc.id, current_point);

        std::pop_heap(c.begin(), c.end(), heap_pred);

        tc = c.back();
    }
}

template <typename ClockType>
template <typename Container>
void timer_service<ClockType>::cancel_timers(Container& c)
{
    for (auto& tc : c)
    {
        cancel_timer(tc);
    }
}


template <typename ClockType>
void timer_service<ClockType>::cancel_timer(const timer_context& tc)
{
    static const operation_aborted_error abort_error{};

    static const time_point null_point{};

    notify(tc.callback, abort_error, tc.id, null_point);
}

template <typename ClockType>
void timer_service<ClockType>::notify(
	const timer_callback& callback,
	const generic_error& e,
    const timer_id& tid,
	const time_point& tp)
{
    port_.post([callback, tid, tp](const generic_error& e)
    {
        callback(e, tid, tp);
    }, e);
}

} // namespace timer

} // namespace cport

#endif //__TIMER_SERVICE_INL__