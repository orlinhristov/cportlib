#ifndef __TASK_HANDLER_BASE_HPP__
#define __TASK_HANDLER_BASE_HPP__

//
// task_handler_base.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <detail/destroyable_obj.hpp>
#include <detail/operation_id.hpp>

namespace mt {

class generic_error;

namespace detail {

class completion_port_impl;
class task_handler_base : public destroyable_obj {
    typedef std::function<
        void(completion_port_impl &port,
             task_handler_base *)> invoke_helper_type;

    typedef std::function<
        void(completion_port_impl &port,
             task_handler_base *,
             const generic_error &)> complete_helper_type;
public:
    void execute(completion_port_impl &port)
    {
        invoke_helper_(port, this);
    }

    void post_complete(completion_port_impl &port, const generic_error &e)
    {
        complete_helper_(port, this, e);
    }

    operation_id id() const
    {
        return id_;
    }
protected:
    task_handler_base(operation_id id,
        invoke_helper_type invoke_helper,
        complete_helper_type complete_helper,
        destroy_helper_type destroy_helper)
        : destroyable_obj(destroy_helper)
        , id_(id)
        , invoke_helper_(invoke_helper)
        , complete_helper_(complete_helper)
    {
    }

    ~task_handler_base()
    {
    }

private:
    operation_id id_;
    invoke_helper_type invoke_helper_;
    complete_helper_type complete_helper_;
};

} // namespace detail

} // namespace mt

#endif // __TASK_HANDLER_BASE_HPP__
