#ifndef __TASK_HANDLER_HPP__
#define __TASK_HANDLER_HPP__

//
// task_handler.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <error_types.hpp>
#include <detail/completion_port_impl.hpp>
#include <detail/obj_mem_pool.hpp>
#include <detail/task_handler_base.hpp>

namespace mt {

namespace detail {

template <typename TaskHandlerType, typename CompletionHandlerType>
class task_handler : public task_handler_base {
    DECLARE_OBJ_MEMORY_POOL(task_handler)
public:
    static task_handler* construct(TaskHandlerType op,
                                   CompletionHandlerType c,
                                   operation_id id)
    {
        return new task_handler(op, c, id);
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
    task_handler(TaskHandlerType op, CompletionHandlerType c, operation_id id)
        : task_handler_base(id,
            task_handler::execute_,
            task_handler::post_complete_,
            task_handler::destroy_)
        , taskHandler_(op)
        , completionHandler_(c)
    {
    }
    
    ~task_handler()
    {
    }

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

} // namespace detail

} // namespace mt

#endif // __TASK_HANDLER_HPP__