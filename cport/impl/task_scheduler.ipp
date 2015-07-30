#ifndef __TASK_SCHEDULER_IPP__
#define __TASK_SCHEDULER_IPP__

//
// task_scheduler.ipp
//
// Copyright (c) 2013-2014 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/task_scheduler.hpp>
#include <cport/completion_port.hpp>

namespace cport {

namespace detail {

inline void default_context(task_scheduler::worker_func_prototype fn)
{
    fn();
}

}

task_scheduler::task_scheduler(completion_port &port, std::size_t concurrency_hint)
    : task_scheduler(port, concurrency_hint, detail::default_context)
{
}

task_scheduler::task_scheduler(completion_port &port, worker_context_prototype wcp)
    : task_scheduler(port, 0, wcp)
{
}

task_scheduler::task_scheduler(completion_port &port,
    std::size_t concurrency_hint, worker_context_prototype wcp)
    : impl_(detail::get_impl(port), concurrency_hint, wcp), cp_(port)
{
}


} // namespace cport

#endif //__TASK_SCHEDULER_IPP__
