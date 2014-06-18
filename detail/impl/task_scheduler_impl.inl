#ifndef __TASK_SCHEDULER_IMPL_INL__
#define __TASK_SCHEDULER_IMPL_INL__

//
// task_scheduler_impl.inl
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cportlib/detail/protected_t.hpp>

namespace mt {

namespace detail {

template <typename WorkerThreadContext>
task_scheduler_impl::task_scheduler_impl(completion_port_impl &port
    , std::size_t concurrency_hint
    , WorkerThreadContext wtc)
    : port_(port)
    , threads_stopped_(false)
{
    if (concurrency_hint == 0)
        concurrency_hint = std::max<std::size_t>(std::thread::hardware_concurrency(), 1);

    typedef detail::protected_t<WorkerThreadContext> WorkerThreadContextP;

    threads_.resize(concurrency_hint);
    for (std::size_t i = 0; i < concurrency_hint; ++i)
    {
        threads_[i] = std::thread(std::bind(
            &task_scheduler_impl::thread_routine<WorkerThreadContextP>,
            this,
            WorkerThreadContextP(wtc)));
    }
}

template <typename TaskHandler, typename CompletionHandler>
inline task_t task_scheduler_impl::async(TaskHandler th, CompletionHandler ch)
{
    typedef task_handler<TaskHandler, CompletionHandler> Wrapper;
    const operation_id id = port_.next_operation_id();
    if (id.valid())
    {
        enqueue_task(Wrapper::construct(th, ch, id));
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
    for (std::thread & t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

template <typename WorkerThreadContext>
inline void task_scheduler_impl::thread_routine(WorkerThreadContext wtc)
{
    wtc(std::bind(&task_scheduler_impl::thread_routine_loop, this));
}

} // namespace detail

} // namespace mt

#endif //__TASK_SCHEDULER_IMPL_INL__
