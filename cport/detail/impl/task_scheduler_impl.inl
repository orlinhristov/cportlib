#ifndef __TASK_SCHEDULER_IMPL_INL__
#define __TASK_SCHEDULER_IMPL_INL__

//
// task_scheduler_impl.inl
//
// Copyright (c) 2013-2014 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/detail/protected_t.hpp>

namespace cport {

namespace detail {

template <typename TaskHandler, typename CompletionHandler>
inline task_t task_scheduler_impl::async(TaskHandler&& th, CompletionHandler&& ch)
{
    const operation_id id = port_.next_operation_id();
    if (id.valid())
    {
        enqueue_task(create_task_handler(std::forward<TaskHandler>(th),
            std::forward<CompletionHandler>(ch), id));
    }
    return task_t(id);
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

inline completion_port_impl& task_scheduler_impl::get_completion_port()
{
    return port_;
}

inline void task_scheduler_impl::cancel_pending_task(task_handler_base *h,
    const generic_error &e)
{
#ifdef CPORT_ENABLE_TASK_STATUS
    h->id().set_status(completion_status::canceled);
#endif
    h->post_complete(port_, e);
}

inline void task_scheduler_impl::stop_threads()
{
    std::unique_lock<std::mutex> lock(guard_);
    threads_stopped_ = true;
    cond_.notify_all();
}

inline void task_scheduler_impl::join_threads()
{
    threads_.join();
}

inline void task_scheduler_impl::thread_routine(worker_context_prototype wtp)
{
    wtp(std::bind(&task_scheduler_impl::thread_routine_loop, this));
}

} // namespace detail

} // namespace cport

#endif //__TASK_SCHEDULER_IMPL_INL__
