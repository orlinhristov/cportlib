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

task_scheduler::task_scheduler(completion_port &port, std::size_t concurrency_hint)
    : impl_(detail::get_impl(port), concurrency_hint, detail::default_context)
{
}

} // namespace cport

#endif //__TASK_SCHEDULER_IPP__
