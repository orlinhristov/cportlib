#ifndef __TASK_SCHEDULER_HPP__
#define __TASK_SCHEDULER_HPP__

//
// task_scheduler.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/config.hpp>
#include <cport/task_t.hpp>
#include <cport/detail/task_scheduler_impl.hpp>
#include <cport/detail/impl_accessor.hpp>
#include <type_traits>

namespace cport {

class completion_port;

/// A class used to schedule operations (tasks) for asynchronous execution.
/**
 * Every object of this type will start as many worker threads as configured.
 * The default number of workers is equal to the number of concurrent
 *  threads supported by the system.
 */
class task_scheduler {
public:
    /// The implementation type
    typedef detail::task_scheduler_impl impl_type;

    /// The prototype of the worker threads entry point 
    typedef impl_type::worker_func_prototype worker_func_prototype;

    /// The prototype of the worker's context
    typedef impl_type::worker_context_prototype worker_context_prototype;

    /// Construct new task_scheduler object.
    /**
     * @param port A port to use to dispatch completion handlers.
     *
     * @param concurrency_hint A number of worker threads to run.
     *  0 == number of concurrent threads supported by the system.
     */
    CPORT_DECL_TYPE explicit task_scheduler(completion_port &port
        , std::size_t concurrency_hint = 0);

    /// Construct new task_scheduler object.
    /**
     * The number of workers started is equal to the number of concurrent
     *  threads supported by the system.
     *
     * @param port A port to use to dispatch completion handlers.
     *
     * @param wcp A context of each worker thread.
     *  This object accepts parameter of type worker_func_prototype
     *  and must call it to run the worker's entry point.
     */
    CPORT_DECL_TYPE task_scheduler(completion_port &port, worker_context_prototype wcp);

    /// Construct new task_scheduler object.
    /**
    * @param port A port to use to dispatch completion handlers.
    *
    * @param concurrency_hint A number of worker threads to run.
    *  0 = number of concurrent threads supported by the system.
    *
    * @param wcp A context of each worker thread.
    *  This object accepts parameter of type worker_func_prototype
    *  and must call it to run the worker's entry point.
    */
    CPORT_DECL_TYPE task_scheduler(completion_port &port
        , std::size_t concurrency_hint, worker_context_prototype wcp);

    /// Disable copy constructor.
    task_scheduler(const task_scheduler&) = delete;

    /// Disable assignment operator.
    task_scheduler& operator=(task_scheduler&) = delete;

    /// Get the completion port used to initialize the object
    completion_port& port() const;

    /// Schedule a task for an asynchronous execution and return immediately.
    /**
     * The completion handler is posted to the completion port after task
     *  execution completes
     *
     * @param th A task handler to be executed asynchronously.
     *
     * @param ch A completion handler to be posted to the completion port
     *  after task execution completes
     *
     * @returns A task identifier
     */
    template <typename TaskHandler, typename CompletionHandler>
    task_t async(TaskHandler&& th, CompletionHandler&& ch);

    /// Schedule a task for an asynchronous execution and return immediately.
    /**
     *
     * @param h A task handler to be executed asynchronously.
     *
     * @returns A task identifier
     */
    template <typename Handler>
    task_t async(Handler&& h);

    /// Cancel specific task.
    /**
     * Completion handler is called with operation_aborted error code.
     *  The method call has no effect if task's handler is already executed.
     *
     * @param task An identifier to a task to be canceled.
     *
     * @returns true if the task associated with the task identifier
     *  was canceled.
     */
    bool cancel(const task_t &task);

    /// Cancel all outstanding tasks.
    /**
     * Completion handler of each task is called
     *  with operation_aborted error code.
     *
     * @returns The number of tasks canceled.
     */
    std::size_t cancel_all();

    /// Get the number of outstanding tasks. Tasks that are currently
    ///  executing are not included.
    std::size_t packaged_tasks() const;

protected:
    /// Get a const reference to the implementation type
    const impl_type& impl() const;

    /// Get a reference to the implementation type
    impl_type& impl();
private:
    template<typename T>
    friend const typename T::impl_type& detail::get_impl(const T&);

    template<typename T>
    friend typename T::impl_type& detail::get_impl(T&);
    impl_type impl_;

    completion_port& cp_;
};

} // namespace cport

#include <cport/impl/task_scheduler.inl>
#ifdef CPORT_HEADER_ONLY_LIB
#include <cport/impl/task_scheduler.ipp>
#endif//CPORT_HEADER_ONLY_LIB

#endif//__TASK_SCHEDULER_HPP__
