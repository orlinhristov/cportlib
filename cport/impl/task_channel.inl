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

#include <cport/task_channel.hpp>
#include <cport/task_scheduler.hpp>
#include <cport/detail/impl_accessor.hpp>
#include <cport/detail/task_handler_base.hpp>
#include <type_traits>
#include <iterator>

namespace cport {

inline task_channel::shared_ptr task_channel::make_shared(task_scheduler &ts)
{
    return shared_ptr(new task_channel(ts));
}

template <typename TaskHandler, typename CompletionHandler>
inline task_t task_channel::enqueue_front(TaskHandler&& th, CompletionHandler&& ch)
{
    std::front_insert_iterator<task_deque> iter(pending_tasks_);
    return enqueue_task(std::forward<TaskHandler>(th), std::forward<CompletionHandler>(ch), iter);
}

template <typename TaskHandler, typename CompletionHandler>
inline task_t task_channel::enqueue_back(TaskHandler&& th, CompletionHandler&& ch)
{
    std::back_insert_iterator<task_deque> iter(pending_tasks_);
    return enqueue_task(std::forward<TaskHandler>(th), std::forward<CompletionHandler>(ch), iter);
}

template <typename Handler>
inline task_t task_channel::enqueue_front(Handler&& h)
{
    return enqueue_front(std::forward<Handler>(h), detail::null_handler_t());
}

template <typename Handler>
inline task_t task_channel::enqueue_back(Handler&& h)
{
    return enqueue_back(std::forward<Handler>(h), detail::null_handler_t());
}

inline std::size_t task_channel::enqueued_tasks() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return pending_tasks_.size();
}

template <typename TaskHandler, typename CompletionHandler, typename InsertIterator>
inline task_t task_channel::enqueue_task(TaskHandler&& th, CompletionHandler&& ch, InsertIterator& it)
{
    detail::task_scheduler_impl& ts = detail::get_impl(ts_);
    detail::completion_port_impl& port = ts.get_completion_port();
    const detail::operation_id opid(port.next_operation_id());
    if (opid.valid())
    {
        typedef detail::protected_t<typename std::remove_reference<TaskHandler>::type> TaskHandlerP;
        typedef detail::protected_t<typename std::remove_reference<CompletionHandler>::type> CompletionHandlerP;
        auto* wrapper = detail::create_task_handler(
            std::bind(&task_channel::task_handler_proxy<TaskHandlerP>,
                shared_from_this(),
                placeholders::error,
                TaskHandlerP(std::forward<TaskHandler>(th))),
            std::bind(&task_channel::completion_handler_proxy<CompletionHandlerP>,
                shared_from_this(),
                placeholders::error,
                CompletionHandlerP(std::forward<CompletionHandler>(ch))),
            opid);

        std::unique_lock<std::mutex> lock(mutex_);
        if (current_task_) 
            *it++ = wrapper;
        else
            enqueue_task(wrapper, lock);
    }
    return task_t(opid);
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

} // namespace cport

#endif // __TASK_CHANNEL_INL__
