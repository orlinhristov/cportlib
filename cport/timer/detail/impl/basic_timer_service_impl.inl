#ifndef __BASIC_TIMER_SERVICE_IMPL_INL__
#define __BASIC_TIMER_SERVICE_IMPL_INL__

#include <cport/timer/detail/basic_timer_service_impl.hpp>

namespace cport {

namespace timer {

namespace detail {

template <typename ClockType>
basic_timer_service_impl<ClockType>::basic_timer_service_impl(
    completion_port& port)
    : port_(port)
{
    timer_thread_ = std::move(
        std::thread(
            std::bind(
                &basic_timer_service_impl::timer_thread_routine,
                this)));
}

template <typename ClockType>
basic_timer_service_impl<ClockType>::~basic_timer_service_impl()
{
    std::unique_lock<std::mutex> lock(guard_);

    stop_thread_ = true;

    cond_var_.notify_all();

    lock.unlock();

    if (timer_thread_.joinable())
    {
        timer_thread_.join();
    }

    release_completion_port();
}

template <typename ClockType>
timer_id basic_timer_service_impl<ClockType>::add_timer(
    time_unit interval,
    timer_callback callback)
{
    const auto next_timer_id = get_next_timer_id();

    std::unique_lock<std::mutex> lock(guard_);

    new_timers_.emplace_back(next_timer_id, interval, callback);

    new_timers_.back().next_point = interval + clock::now();

    block_completion_port();

    cond_var_.notify_all();

    return next_timer_id;
}

template <typename ClockType>
void basic_timer_service_impl<ClockType>::remove_timer(
    timer_id id,
    bool notify)
{
    auto success = false;

    std::unique_lock<std::mutex> lock(guard_);

    del_timers_.emplace_back(id, notify);

    cond_var_.notify_all();
}


template <typename ClockType>
void basic_timer_service_impl<ClockType>::block_completion_port()
{
    if (!port_release_wrapper_)
    {
        port_release_wrapper_ = std::move(
            wrap_completion_handler(
                cport::detail::null_handler_t{}, port_
            )
        );
    }
}

template <typename ClockType>
void basic_timer_service_impl<ClockType>::release_completion_port()
{
    if (port_release_wrapper_)
    {
        // Release all threads waiting for completion
        // if there is not timers
        port_release_wrapper_();
    }
}

template <typename ClockType>
void basic_timer_service_impl<ClockType>::timer_thread_routine()
{
    std::vector<timer_context> timer_heap;

    for (;;)
    {
        std::unique_lock<std::mutex> lock(guard_);

        if (!stop_thread_ && new_timers_.empty()
            && del_timers_.empty() && resume_timers_.empty())
        {
            // Check if there is not timers or all are paused
            if (timer_heap.empty() || timer_heap.back().paused)
            {
                if (timer_heap.empty())
                {
                    release_completion_port();
                }
                
                // Block the thread until notified for some event
                cond_var_.wait(lock);
            }
            else
            {
                // We have at least one timer to watch on!
                const auto& next_timer = timer_heap.back();

                // Wait until the most recent time occurs or some event is set
                cond_var_.wait_until(lock, next_timer.next_point);
            }
        }

        if (stop_thread_)
        {
            cancel_timers(timer_heap);

            break;
        }

        add_timers(timer_heap);

        del_timers(timer_heap);

        resume_timers(timer_heap);

        lock.unlock();

        check_ready_timers(timer_heap);
    }
}

template <typename ClockType>
template <typename Container>
void basic_timer_service_impl<ClockType>::add_timers(Container& c)
{
    for (const auto& new_timer : new_timers_)
    {
        c.push_back(new_timer);
    }

    new_timers_.clear();
}

template <typename ClockType>
template <typename Container>
void basic_timer_service_impl<ClockType>::del_timers(Container& c)
{
    if (!del_timers_.empty())
    {
        const auto it = std::remove_if(c.begin(),
            c.end(), [this](timer_context& ctxt)
        {
            for (const auto& del_timer : del_timers_)
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
void basic_timer_service_impl<ClockType>::resume_timers(Container& c)
{
    for (const auto& tid : resume_timers_)
    {
        for (auto i = c.rbegin(), e = c.rend(); i != e; ++i)
        {
            if (i->id == tid)
            {
                assert(i->paused);

                // Calc the next time point at which the timer has to be notified
                const auto& last_point = i->next_point;

                const auto& current_point = clock::now();

                const auto multiplier = int((current_point - last_point) / i->interval);

                i->next_point += i->interval * multiplier;

                if (i->next_point < current_point)
                {
                    i->next_point += i->interval;
                }

                assert(i->next_point >= current_point);

                i->paused = false;

                break;
            }
        }
    }

    resume_timers_.clear();
}

template <typename ClockType>
template <typename Container>
void basic_timer_service_impl<ClockType>::check_ready_timers(Container& c)
{
    static const auto heap_pred = [](const timer_context& a, const timer_context& b)
    {
        if (a.paused != b.paused)
        {
            return a.paused ? true : false;
        }

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

    while (!tc.paused && current_point >= tc.next_point)
    {
        invoke_callback_indirect(tc, generic_error{}, current_point);

        std::make_heap(c.begin(), c.end(), heap_pred);

        std::pop_heap(c.begin(), c.end(), heap_pred);

        tc = c.back();
    }

}

template <typename ClockType>
template <typename Container>
void basic_timer_service_impl<ClockType>::cancel_timers(Container& c)
{
    for (auto& tc : c)
    {
        cancel_timer(tc);
    }
}

template <typename ClockType>
void basic_timer_service_impl<ClockType>::cancel_timer(timer_context& tc)
{
    static const operation_aborted_error abort_error{};

    static const time_point null_point{};

    // the mutex should be aquired
    assert(!guard_.try_lock());

    if (stop_thread_)
    {
        invoke_callback_direct(tc, abort_error, null_point);
    }
    else
    {
        invoke_callback_indirect(tc, abort_error, null_point);
    }
}

template <typename ClockType>
void basic_timer_service_impl<ClockType>::invoke_callback_direct(
    timer_context& tc,
    const generic_error& e,
    const time_point& tp)
{
    tc.paused = true;

    tc.callback(e, tc.id, tp);
}

template <typename ClockType>
void basic_timer_service_impl<ClockType>::invoke_callback_indirect(
    timer_context& tc,
    const generic_error& e,
    const time_point& tp)
{
    tc.paused = true;

    auto ptr = this->shared_from_this();

    port_.post(
        [ptr, callback = tc.callback, tid = tc.id, tp](const generic_error& e)
    {
        callback(e, tid, tp);

        std::unique_lock<std::mutex> lock(ptr->guard_);

        ptr->resume_timers_.push_back(tid);

        ptr->cond_var_.notify_all();
    }, e);
}

} // namespace detail

} // namespace timer

} // namespace cport

#endif //__BASIC_TIMER_SERVICE_IMPL_INL__