#ifndef __TASK_HANDLER_HPP__
#define __TASK_HANDLER_HPP__

//
// task_handler.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/error_types.hpp>
#include <cport/detail/completion_port_impl.hpp>
#include <cport/detail/obj_mem_pool.hpp>
#include <cport/detail/task_handler_base.hpp>

namespace cport {

namespace detail {

template <typename TaskHandlerType, typename CompletionHandlerType>
class task_handler : public task_handler_base {
    DECLARE_OBJ_MEMORY_POOL(task_handler)
public:
    task_handler(const TaskHandlerType& op, const CompletionHandlerType& c, const operation_id& id)
        : task_handler_base(id,
            task_handler::execute_,
            task_handler::post_complete_,
            task_handler::destroy_)
        , taskHandler_(op)
        , completionHandler_(c)
    {
    }

    task_handler(TaskHandlerType&& op, CompletionHandlerType&& c, const operation_id& id)
        : task_handler_base(id,
            task_handler::execute_,
            task_handler::post_complete_,
            task_handler::destroy_)
        , taskHandler_(std::move(op))
        , completionHandler_(std::move(c))
    {
    }

    ~task_handler()
    {
    }

    template <typename CompletionPort>
    void execute(CompletionPort &port)
    {
        generic_error e;
        taskHandler_(e);
        post_complete(port, e);
    }

    template <typename CompletionPort>
    void post_complete(CompletionPort &port, const generic_error &e)
    {
        port.post(completionHandler_, id(), e);
    }
   
private:
    typedef task_handler<TaskHandlerType, CompletionHandlerType> this_type;

    static void destroy_(destroyable_obj *base)
    {
        assert(base != nullptr);
        delete static_cast<this_type *>(base);
    }

    static void execute_(completion_port_impl &port, task_handler_base *base)
    {
        assert(base != nullptr);
        static_cast<this_type *>(base)->execute(port);
    }

    static void post_complete_(completion_port_impl &port,
        task_handler_base *base, const generic_error &e)
    {
        assert(base != nullptr);
        static_cast<this_type *>(base)->post_complete(port, e);
    }

    TaskHandlerType taskHandler_;
    CompletionHandlerType completionHandler_;
};

IMPLEMENT_OBJ_MEMORY_POOL_T2(task_handler, TH, CH);

template <typename taskhandlertype, typename completionhandlertype>
inline task_handler<typename std::remove_reference<taskhandlertype>::type,
    typename std::remove_reference<completionhandlertype>::type>*
create_task_handler(taskhandlertype&& th, completionhandlertype&& ch, const operation_id& id)
{
    return new task_handler<typename std::remove_reference<taskhandlertype>::type,
        typename std::remove_reference<completionhandlertype>::type>(
            std::forward<taskhandlertype>(th),
            std::forward<completionhandlertype>(ch),
            id);
}

} // namespace detail

} // namespace cport

#endif // __TASK_HANDLER_HPP__
