#ifndef __TASK_SCHEDULER_INL__
#define __TASK_SCHEDULER_INL__

//
// task_scheduler.inl
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cportlib/detail/null_handler_t.hpp>

namespace mt {

namespace detail {

inline void default_context(task_scheduler::worker_func_prototype fn)
{
    fn();
}

}

template <typename WorkerThreadContext>
inline task_scheduler::task_scheduler(completion_port &port, WorkerThreadContext wtc)
    : impl_(detail::get_impl(port), 0, wtc)
{
}

template <typename WorkerThreadContext>
inline task_scheduler::task_scheduler(completion_port &port,
    std::size_t concurrency_hint,
    WorkerThreadContext wtc)
    : impl_(detail::get_impl(port), concurrency_hint, wtc)
{
}

template <typename TaskHandler, typename CompletionHandler>
inline task_t task_scheduler::async(TaskHandler th, CompletionHandler ch)
{
    return impl().async(th, ch);
}

template <typename Handler>
inline task_t task_scheduler::async(Handler h)
{
    return async(h, detail::null_handler_t());
}

inline bool task_scheduler::cancel(const task_t &task)
{
    return impl().cancel(task);
}

inline std::size_t task_scheduler::cancel_all()
{
    return impl().cancel_all();
}

inline std::size_t task_scheduler::packaged_tasks() const
{
    return impl().packaged_tasks();
}

inline const task_scheduler::impl_type& task_scheduler::impl() const
{
    return impl_;
}

inline task_scheduler::impl_type& task_scheduler::impl()
{
    return impl_;
}

} // namespace mt

#endif //__TASK_SCHEDULER_INL__
