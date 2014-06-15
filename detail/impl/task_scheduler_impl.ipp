#ifndef __TASK_SCHEDULER_IMPL_IPP__
#define __TASK_SCHEDULER_IMPL_IPP__

//
// task_scheduler_impl.ipp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <detail/task_scheduler_impl.hpp>
#include <algorithm>

namespace mt {

namespace detail {

task_scheduler_impl::~task_scheduler_impl()
{
    stop_threads();
    join_threads();
    cancel_pending_tasks();
}

bool task_scheduler_impl::cancel(const task_t &task)
{
    assert(task);

    std::unique_lock<std::mutex> lock(guard_);
    if (pending_tasks_.empty())
        return false;

    // TODO: Optimize
    // Start search from the side with id closer to the passed one
    auto i = std::find_if(pending_tasks_.begin(), pending_tasks_.end(),
            [task](task_handler_base *h){return task_t(h->id()) == task;});

    if (i == pending_tasks_.end())
        return false;

    auto_destroy op(*i);
    pending_tasks_.erase(i);
    lock.unlock();
    cancel_pending_task(op.get());
    return true;
}

inline std::size_t task_scheduler_impl::cancel_all()
{
    std::unique_lock<std::mutex> lock(guard_);
    const std::size_t count = pending_tasks_.size();
    cancel_pending_tasks();
    return count;
}

inline std::size_t task_scheduler_impl::packaged_tasks() const
{
    std::unique_lock<std::mutex> lock(guard_);
    return pending_tasks_.size();
}

inline void task_scheduler_impl::enqueue_task(task_handler_base *h)
{
    std::unique_lock<std::mutex> lock(guard_);
    pending_tasks_.push_back(h);
#ifdef CPORT_ENABLE_TASK_STATUS
    h->id().set_status(completion_status::scheduled);
#endif
    cond_.notify_one();
}

inline void task_scheduler_impl::cancel_pending_task(task_handler_base *h,
    const generic_error &e)
{
#ifdef CPORT_ENABLE_TASK_STATUS
    h->id().set_status(completion_status::canceled);
#endif

    h->post_complete(port_, e);
}

void task_scheduler_impl::cancel_pending_tasks()
{
    const operation_aborted_error e;
    while (!pending_tasks_.empty()) {
        auto_destroy task(pending_tasks_.front());
        pending_tasks_.pop_front();
        cancel_pending_task(task.get(), e);
    }
}

void task_scheduler_impl::stop_threads()
{
    std::unique_lock<std::mutex> lock(guard_);
    threads_stopped_ = true;
    cond_.notify_all();
}

void task_scheduler_impl::join_threads()
{
    for (std::thread & t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void task_scheduler_impl::thread_routine_loop()
{
    for (;;) {
        std::unique_lock<std::mutex> lock(guard_);
        if (threads_stopped_)
            break;

        if (pending_tasks_.empty()) {
            cond_.wait(lock);
        }
        else {
            auto_destroy task(pending_tasks_.front());
            pending_tasks_.pop_front();
            lock.unlock();
#ifdef CPORT_ENABLE_TASK_STATUS
            task->id().set_status(completion_status::executing);
#endif
            task->execute(port_);

#ifdef CPORT_ENABLE_TASK_STATUS
            task->id().set_status(completion_status::complete);
#endif
        }
    }
}

} // namespace detail

} // namespace mt

#endif //__TASK_SCHEDULER_IMPL_IPP__
