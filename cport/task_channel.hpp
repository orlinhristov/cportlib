#ifndef __TASK_CHANNEL_HPP__
#define __TASK_CHANNEL_HPP__

//
// task_channel.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/config.hpp>
#include <cport/task_t.hpp>
#include <cport/placeholders.hpp>
#include <deque>
#include <mutex>
#include <memory>

namespace cport {

namespace detail {
class task_handler_base;
}

class task_scheduler;

class generic_error;

/// This class is a task_scheduler wrapper that guarantees sequential execution of tasks
class task_channel : public std::enable_shared_from_this<task_channel> {

    explicit CPORT_DECL_TYPE task_channel(task_scheduler &ts);
public:

    typedef std::shared_ptr<task_channel> shared_ptr;

    static shared_ptr make_shared(task_scheduler &ts);

    task_channel(const task_channel&) = delete;

    task_channel& operator=(const task_channel&) = delete;

    /// Add a task before the current first task.
    /**
    * The completion handler is posted to the completion port
    * after task execution completes.
    *
    * @param th A task handler to be executed asynchronously.
    *
    * @param ch A completion handler to be posted to the completion port
    *  after task execution completes
    *
    * @returns A task identifier
    */
    template <typename TaskHandler, typename CompletionHandler>
    task_t enqueue_front(TaskHandler&& th, CompletionHandler&& ch);

    /// Add a task after the current last task.
    /**
    * The completion handler is posted to the completion port
    * after task execution completes.
    *
    * @param th A task handler to be executed asynchronously.
    *
    * @param ch A completion handler to be posted to the completion port
    *  after task execution completes
    *
    * @returns A task identifier
    */
    template <typename TaskHandler, typename CompletionHandler>
    task_t enqueue_back(TaskHandler&& th, CompletionHandler&& ch);

    /// Add a task before the current first task.
    /**
    *
    * @param h A task handler to be executed asynchronously.
    *
    * @returns A task identifier
    */
    template <typename Handler>
    task_t enqueue_front(Handler&& h);

    /// Add a task after the current last task.
    /**
    *
    * @param h A task handler to be executed asynchronously.
    *
    * @returns A task identifier
    */
    template <typename Handler>
    task_t enqueue_back(Handler&& h);

    /// Cancel specific task.
    /**
    * Completion handler is called with operation_aborted error code.
    * The method call has no effect if the task's handler is already executed.
    *
    * @param task An identifier to a task to be canceled.
    *
    * @returns true if the task associated with the task identifier
    *  was canceled.
    */
    CPORT_DECL_TYPE bool cancel(const task_t &task);

    /// Cancel all outstanding tasks.
    /**
    * Completion handler of each task is called
    * with operation_aborted error code.
    *
    * @returns The number of tasks canceled.
    */
    CPORT_DECL_TYPE std::size_t cancel_all();

    /// Return the number of outstanding tasks scheduled through this channel.
    /// Tasks that are currently executing are not included.
    std::size_t enqueued_tasks() const;
private:

    template <typename TaskHandler, typename CompletionHandler, typename InsertIterator>
    task_t enqueue_task(TaskHandler&& th, CompletionHandler&& ch, InsertIterator& it);

    CPORT_DECL_TYPE void enqueue_task(detail::task_handler_base *h, std::unique_lock<std::mutex> &lock);

    CPORT_DECL_TYPE void complete_task(detail::task_handler_base *h, std::unique_lock<std::mutex> &lock);

    CPORT_DECL_TYPE void enqueue_next_task();

    template <typename Handler>
    void task_handler_proxy(generic_error &e, Handler h);

    template <typename Handler>
    void completion_handler_proxy(const generic_error &e, Handler h);

    mutable std::mutex mutex_;
    typedef std::deque<detail::task_handler_base *> task_deque;
    task_deque pending_tasks_;
    task_deque canceled_tasks_;
    task_t current_task_;
    task_scheduler &ts_;
};

} // namespace cport

#include <cport/impl/task_channel.inl>
#ifdef CPORT_HEADER_ONLY_LIB
#include <cport/impl/task_channel.ipp>
#endif//CPORT_HEADER_ONLY_LIB

#endif //__TASK_CHANNEL_HPP__
