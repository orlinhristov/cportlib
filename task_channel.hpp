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

#include <config.hpp>
#include <task_t.hpp>
#include <placeholders.hpp>
#include <detail/impl_accessor.hpp>
#include <detail/task_handler_base.hpp>
#include <deque>
#include <mutex>
#include <memory>

namespace mt {

class task_scheduler;
/// This class is a task_scheduler wrapper that guarantees sequential execution of tasks
class task_channel : public std::enable_shared_from_this<task_channel> {

    explicit CPORT_DECL_TYPE task_channel(task_scheduler &ts);
public:

    typedef std::shared_ptr<task_channel> shared_ptr;

    static shared_ptr make_shared(task_scheduler &ts);

    task_channel(const task_channel&) = delete;

    task_channel& operator=(const task_channel&) = delete;

    /// Enqueue a task for an asynchronous execution
    /**
    * The completion handler is posted to the completion port after task
    * execution completes
    *
    * @param th A task handler to be executed asynchronously.
    *
    * @param ch A completion handler to be posted to the completion port
    *  after task execution completes
    *
    * @returns A task identifier
    */
    template <typename TaskHandler, typename CompletionHandler>
    task_t enqueue(TaskHandler th, CompletionHandler ch);

    /// Enqueue a task for an asynchronous execution
    /**
    *
    * @param h A task handler to be executed asynchronously.
    *
    * @returns A task identifier
    */
    template <typename Handler>
    task_t enqueue(Handler h);

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
    CPORT_DECL_TYPE std::size_t enqueued_tasks() const;
private:
    struct find_task_pred {
        find_task_pred& operator=(const find_task_pred&) = delete;

        explicit find_task_pred(const task_t &t)
            : task(t)
        {
        }

        bool operator() (const detail::task_handler_base *h) const
        {
            return task_t(h->id()) == task;
        }

        const task_t& task;
    };

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

template <typename TaskHandler, typename CompletionHandler>
task_t task_channel::enqueue(TaskHandler th, CompletionHandler ch)
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

#ifdef CPORT_HEADER_ONLY_LIB
#include <impl/task_channel.ipp>
#endif//CPORT_HEADER_ONLY_LIB

#endif //__TASK_CHANNEL_HPP__