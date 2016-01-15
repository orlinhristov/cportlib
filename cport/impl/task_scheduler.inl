#ifndef __TASK_SCHEDULER_INL__
#define __TASK_SCHEDULER_INL__

//
// task_scheduler.inl
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/detail/null_handler_t.hpp>

namespace cport {

inline completion_port& task_scheduler::port()
{
    return cp_;
}

template <typename TaskHandler, typename CompletionHandler>
inline task_t task_scheduler::async(TaskHandler&& th, CompletionHandler&& ch)
{
    return impl().async(std::forward<TaskHandler>(th), std::forward<CompletionHandler>(ch));
}

template <typename Handler>
inline task_t task_scheduler::async(Handler&& h)
{
    return async(std::forward<Handler>(h), detail::null_handler_t());
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

} // namespace cport

#endif //__TASK_SCHEDULER_INL__
