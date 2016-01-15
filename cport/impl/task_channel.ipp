#ifndef __TASK_CHANNEL_IPP__
#define __TASK_CHANNEL_IPP__

//
// task_channel.ipp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/task_channel.hpp>
#include <cport/task_scheduler.hpp>
#include <iterator>

namespace cport {

namespace detail {

struct find_task_pred {

	find_task_pred& operator=(const find_task_pred&) = delete;

    explicit find_task_pred(const task_t &t)
        : task(t)
    {
    }

    bool operator() (const detail::task_handler_base *h) const
    {
        return task_t(h->id()) == task;
    }

    const task_t& task;
};

} // namespace detail

task_channel::task_channel(task_scheduler &ts)
: ts_(ts)
{
}

bool task_channel::cancel(const task_t &task)
{
    bool canceled = false;

    std::unique_lock<std::mutex> lock(mutex_);

    if (task == current_task_)
    {
        canceled = std::find_if(canceled_tasks_.begin()
			, canceled_tasks_.end(), detail::find_task_pred(task)) != canceled_tasks_.end();

        if (!canceled)
        {
            canceled = ts_.cancel(task);
        }
    }
    else if (!pending_tasks_.empty())
    {
        assert(current_task_);

        // We use iterator here as erase on const_iterator not implemented before g++ v.4.9.0.
        const task_deque::iterator it = std::find_if(pending_tasks_.begin()
			, pending_tasks_.end(), detail::find_task_pred(task));

        if (it != pending_tasks_.end())
        {
            canceled_tasks_.push_back(*it);
            pending_tasks_.erase(it);
            canceled = true;
        }
    }
    return canceled;
}

std::size_t task_channel::cancel_all()
{
    std::size_t count = 0;

    std::unique_lock<std::mutex> lock(mutex_);

    if (current_task_)
    {
        if (std::find_if(canceled_tasks_.begin(), canceled_tasks_.end()
			, detail::find_task_pred(current_task_)) != canceled_tasks_.end())
        {
            ts_.cancel(current_task_);
            ++count;
        }
    }

    std::copy(pending_tasks_.begin(), pending_tasks_.end()
        , std::back_insert_iterator<task_deque>(canceled_tasks_));

    count += pending_tasks_.size();
    pending_tasks_.clear();
    return count;
}

void task_channel::enqueue_task(detail::task_handler_base *h, std::unique_lock<std::mutex> &lock)
{
    assert(lock.owns_lock());
    current_task_ = task_t(h->id());
    lock.unlock();
    detail::get_impl(ts_).enqueue_task(h);
}

void task_channel::complete_task(detail::task_handler_base *h, std::unique_lock<std::mutex> &lock)
{
    assert(lock.owns_lock());
    current_task_ = task_t(h->id());
    lock.unlock();
    h->post_complete(detail::get_impl(ts_).get_completion_port(), operation_aborted_error());
}

void task_channel::enqueue_next_task()
{
    // First handle canceled tasks
    std::unique_lock<std::mutex> lock(mutex_);

    if (!canceled_tasks_.empty())
    {
        detail::task_handler_base *h = canceled_tasks_.front();
        canceled_tasks_.pop_front();
        complete_task(h, lock);
        return;
    }

    if (!pending_tasks_.empty())
    {
        detail::task_handler_base *h = pending_tasks_.front();
        pending_tasks_.pop_front();
        enqueue_task(h, lock);
        return;
    }

    current_task_ = task_t();
}

} // namespace cport

#endif // __TASK_CHANNEL_IPP__
