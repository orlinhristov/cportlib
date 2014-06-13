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
#include <completion_port.hpp>
#include <error_types.hpp>
#include <task_t.hpp>
#include <detail/task_handler.hpp>
#include <detail/protected_t.hpp>
#include <condition_variable>
#include <memory>
#include <deque>
#include <thread>
#include <functional>
#include <algorithm>

namespace mt {

namespace detail {

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

	CPORT_DECL_TYPE std::size_t cancel_all();

	CPORT_DECL_TYPE std::size_t packaged_tasks() const;

	CPORT_DECL_TYPE void enqueue_task(task_handler_base *h);

    completion_port_impl& get_completion_port()
    {
        return port_;
    }

private:
    void cancel_pending_task(task_handler_base *h
        , const generic_error &e = operation_aborted_error());
    
    CPORT_DECL_TYPE void cancel_pending_tasks();

    CPORT_DECL_TYPE void stop_threads();

    CPORT_DECL_TYPE void join_threads();

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

template <typename WorkerThreadContext>
task_scheduler_impl::task_scheduler_impl(completion_port_impl &port
	, std::size_t concurrency_hint
	, WorkerThreadContext wtc)
	: port_(port)
	, threads_stopped_(false)
{
	if (concurrency_hint == 0)
		concurrency_hint = std::max<std::size_t>(std::thread::hardware_concurrency(), 1);

	typedef detail::protected_t<WorkerThreadContext> WorkerThreadContextP;

	threads_.resize(concurrency_hint);
	for (std::size_t i = 0; i < concurrency_hint; ++i)
	{
		threads_[i] = std::thread(std::bind(
			&task_scheduler_impl::thread_routine<WorkerThreadContextP>,
			this,
			WorkerThreadContextP(wtc)));
	}
}

template <typename TaskHandler, typename CompletionHandler>
inline task_t task_scheduler_impl::async(TaskHandler th, CompletionHandler ch)
{
	typedef task_handler<TaskHandler, CompletionHandler> Wrapper;
	const operation_id id = port_.next_operation_id();
	if (id.valid())
	{
		enqueue_task(Wrapper::construct(th, ch, id));
	}
	return task_t(id);
}

template <typename WorkerThreadContext>
inline void task_scheduler_impl::thread_routine(WorkerThreadContext wtc)
{
	wtc(std::bind(&task_scheduler_impl::thread_routine_loop, this));
}

} // namespace detail

} // namespace mt

#ifdef CPORT_HEADER_ONLY_LIB
#include <detail/impl/task_scheduler_impl.ipp>
#endif//CPORT_HEADER_ONLY_LIB

#endif//__TASK_SCHEDULER_IMPL_HPP__