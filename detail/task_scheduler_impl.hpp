#ifndef __TASK_SCHEDULER_IMPL_HPP__
#define __TASK_SCHEDULER_IMPL_HPP__

//
// task_scheduler_impl.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <config.hpp>
#include <error_types.hpp>
#include <task_t.hpp>
#include <detail/task_handler.hpp>
#include <condition_variable>
#include <algorithm>
#include <deque>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace mt {

namespace detail {

class completion_port_impl;

class task_scheduler_impl {
    typedef std::unique_ptr<task_handler_base, destroyable_deletor> auto_destroy;
public:
    typedef std::function<void (void)> worker_func_prototype;

    template <typename WorkerThreadContext>
    task_scheduler_impl(completion_port_impl &port
        , std::size_t concurrency_hint
        , WorkerThreadContext wtc);

    CPORT_DECL_TYPE ~task_scheduler_impl();

    template <typename TaskHandler, typename CompletionHandler>
    task_t async(TaskHandler th, CompletionHandler ch);

    CPORT_DECL_TYPE bool cancel(const task_t &task);

    std::size_t cancel_all();

    std::size_t packaged_tasks() const;

    void enqueue_task(task_handler_base *h);

    completion_port_impl& get_completion_port();

private:
    void cancel_pending_task(task_handler_base *h
        , const generic_error &e = operation_aborted_error());
    
    CPORT_DECL_TYPE void cancel_pending_tasks();

    void stop_threads();

    void join_threads();

    template <typename WorkerThreadContext>
    void thread_routine(WorkerThreadContext wtc);

    CPORT_DECL_TYPE void thread_routine_loop();

    completion_port_impl &port_;
    std::vector<std::thread> threads_;
    mutable std::mutex guard_;
    std::deque<task_handler_base *> pending_tasks_;
    std::condition_variable cond_;
    bool threads_stopped_;
};

} // namespace detail

} // namespace mt

#include <detail/impl/task_scheduler_impl.inl>
#ifdef CPORT_HEADER_ONLY_LIB
#include <detail/impl/task_scheduler_impl.ipp>
#endif//CPORT_HEADER_ONLY_LIB

#endif//__TASK_SCHEDULER_IMPL_HPP__