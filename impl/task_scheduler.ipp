#ifndef __TASK_SCHEDULER_IPP__
#define __TASK_SCHEDULER_IPP__

//
// task_scheduler.ipp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <task_scheduler.hpp>

namespace mt {

namespace detail {

inline void default_context(task_scheduler::worker_func_prototype fn)
{
    fn();
}

}

task_scheduler::task_scheduler(completion_port &port, std::size_t concurrency_hint)
: impl_(detail::get_impl(port), concurrency_hint, detail::default_context)
{
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

#endif //__TASK_SCHEDULER_IPP__