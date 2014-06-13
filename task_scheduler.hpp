#ifndef __TASK_SCHEDULER_HPP__
#define __TASK_SCHEDULER_HPP__

//
// task_scheduler.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <config.hpp>
#include <task_t.hpp>
#include <detail/task_scheduler_impl.hpp>
#include <detail/impl_accessor.hpp>

namespace mt {

class completion_port;
class task_scheduler {
public:
    /// The implementation type
    typedef detail::task_scheduler_impl impl_type;

    /// The prototype of the worker threads entry point 
    typedef impl_type::worker_func_prototype worker_func_prototype;

    /// Construct new task_scheduler object.
    /**
     * @param port A port to use to dispatch completion handlers.
     *
     * @param concurrency_hint A number of worker threads to run.
     *  0 == number of concurrent threads supported by the system.
     */
	CPORT_DECL_TYPE task_scheduler(completion_port &port, std::size_t concurrency_hint = 0);

    /// Construct new task_scheduler object.
    /**
     * The number of workers started is equal to the number of concurrent
     * threads supported by the system.
     *
     * @param port A port to use to dispatch completion handlers.
     *
     * @param wtc Callable object called at start of each worker thread.
     *  This object must accept parameter of type worker_func_prototype
     *  and must call it to run the worker's entry point.
     */
    template <typename WorkerThreadContext>
    task_scheduler(completion_port &port, WorkerThreadContext wtc);

    /// Construct new task_scheduler object.
    /**
    * @param port A port to use to dispatch completion handlers.
    *
    * @param concurrency_hint A number of worker threads to run.
    *  0 = number of concurrent threads supported by the system.
    *
    * @param wtc Callable object called in the context of each worker thread.
    *  This object must accept parameter of type worker_func_prototype
    *  and must call it to run the worker's entry point.
    */
    template <typename WorkerThreadContext>
    task_scheduler(completion_port &port
        , std::size_t concurrency_hint, WorkerThreadContext wtc);

    /// Disable copy constructor.
    task_scheduler(const task_scheduler&) = delete;

    /// Disable assignment operator.
    task_scheduler& operator=(task_scheduler&) = delete;

    /// Schedule a task for an asynchronous execution
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
    task_t async(TaskHandler th, CompletionHandler ch);

    /// Schedule a task for an asynchronous execution
    /**
     *
     * @param h A task handler to be executed asynchronously.
     *
     * @returns A task identifier
     */
    template <typename Handler>
    task_t async(Handler h);

    /// Cancel specific task.
    /**
     * Completion handler is called with operation_aborted error code.
     * The method call has no effect if task's handler is already executed.
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

    /// Return the number of outstanding tasks. Tasks that are currently
    /// executing are not included.
    std::size_t packaged_tasks() const;

protected:
	CPORT_DECL_TYPE const impl_type& impl() const;
	CPORT_DECL_TYPE impl_type& impl();
private:
    template<typename T>
    friend const typename T::impl_type& detail::get_impl(const T&);

    template<typename T>
    friend typename T::impl_type& detail::get_impl(T&);
    impl_type impl_;
};

template <typename WorkerThreadContext>
task_scheduler::task_scheduler(completion_port &port, WorkerThreadContext wtc)
    : impl_(detail::get_impl(port), 0, wtc)
{
}

template <typename WorkerThreadContext>
task_scheduler::task_scheduler(completion_port &port,
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

} // namespace mt

#ifdef CPORT_HEADER_ONLY_LIB
#include <impl/task_scheduler.ipp>
#endif//CPORT_HEADER_ONLY_LIB

#endif//__TASK_SCHEDULER_HPP__
