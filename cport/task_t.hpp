#ifndef __TASK_T_HPP__
#define __TASK_T_HPP__

//
// task_t.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/detail/operation_id.hpp>
#include <utility>

namespace cport {

class task_t {
public:
    typedef detail::operation_id value_type;

    explicit task_t(value_type id = value_type())
        : id_(id)
    {
    }

    task_t(const task_t& t) = default;

    task_t(task_t&& t)
        : id_(std::move(t.id_))
    {
    }

    task_t& operator=(const task_t& t) = default;

    task_t& operator=(task_t&& t)
    {
        if (this != &t)
        {
            id_ = std::move(t.id_);
        }
        return *this;
    }

    bool operator==(const task_t &t) const
    {
        return id_ == t.id_;
    }

    bool operator!=(const task_t &t) const 
    {
        return !(*this == t);
    }

    explicit operator bool() const
    {
        return *this != task_t();
    }

#ifdef CPORT_ENABLE_TASK_STATUS
    completion_status get_status() const
    {
        return id_.get_status();
    }

    void wait()
    {
        id_.wait();
    }
#endif

private:
    value_type id_;
};

}

#endif//__TASK_T_HPP__
