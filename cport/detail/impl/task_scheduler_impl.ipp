#ifndef __TASK_SCHEDULER_IMPL_IPP__
#define __TASK_SCHEDULER_IMPL_IPP__

//
// task_scheduler_impl.ipp
//
// Copyright (c) 2013-2014 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/detail/task_scheduler_impl.hpp>
#include <algorithm>

namespace cport {

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

void task_scheduler_impl::cancel_pending_tasks()
{
    const operation_aborted_error e;
    while (!pending_tasks_.empty()) {
        auto_destroy task(pending_tasks_.front());
        pending_tasks_.pop_front();
        cancel_pending_task(task.get(), e);
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

} // namespace cport

#endif //__TASK_SCHEDULER_IMPL_IPP__
