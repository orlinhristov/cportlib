#ifndef __TASK_CHANNEL_INL__
#define __TASK_CHANNEL_INL__

//
// task_channel.inl
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <task_channel.hpp>
#include <task_scheduler.hpp>
#include <detail/impl_accessor.hpp>
#include <detail/task_handler_base.hpp>
#include <iterator>

namespace mt {

inline task_channel::shared_ptr task_channel::make_shared(task_scheduler &ts)
{
    return shared_ptr(new task_channel(ts));
}

template <typename TaskHandler, typename CompletionHandler>
inline task_t task_channel::enqueue(TaskHandler th, CompletionHandler ch)
{
    typedef detail::protected_t<TaskHandler> TaskHandlerP;
    typedef detail::protected_t<CompletionHandler> CompletionHandlerP;
    detail::task_scheduler_impl& ts = detail::get_impl(ts_);
    detail::completion_port_impl& port = ts.get_completion_port();
    const detail::operation_id opid(port.next_operation_id());
    if (opid.valid()) {
        auto thp = std::bind(&task_channel::task_handler_proxy<TaskHandlerP>
            , shared_from_this()
            , placeholders::error
            , TaskHandlerP(th));

        auto chp = std::bind(&task_channel::completion_handler_proxy<CompletionHandlerP>
            , shared_from_this()
            , placeholders::error
            , CompletionHandlerP(ch));

        detail::task_handler_base *w =
            detail::task_handler<decltype(thp), decltype(chp)>::construct(thp, chp, opid);

        std::unique_lock<std::mutex> lock(mutex_);
        if (current_task_) {
            pending_tasks_.push_back(w);
        }
        else {
            enqueue_task(w, lock);
        }
    }
    return task_t(opid);
}

template <typename Handler>
inline task_t task_channel::enqueue(Handler h)
{
    return enqueue(h, detail::null_handler_t());
}

inline std::size_t task_channel::enqueued_tasks() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return pending_tasks_.size();
}

template <typename Handler>
inline void task_channel::task_handler_proxy(generic_error &e, Handler h)
{
    h(e);
}

template <typename Handler>
inline void task_channel::completion_handler_proxy(const generic_error &e, Handler h)
{
    h(e);
    enqueue_next_task();
}

} // namespace mt

#endif // __TASK_CHANNEL_INL__
