#ifndef __TASK_T_HPP__
#define __TASK_T_HPP__

//
// task_t.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/detail/operation_id.hpp>
#include <utility>

namespace cport {

/// This class encapsulates an operation identifier assigned to a scheduled task.
class task_t {
public:
    typedef detail::operation_id value_type;

    /// Construct a task_t object from an operation identifier
    /**
     * @param id An operation identifier. An invalid idetifier is assigned
     *  if no parameter was passed.
     */
    explicit task_t(value_type id = value_type())
        : id_(id)
    {
    }

    /// Construct a copy of t
    task_t(const task_t& t) = default;

    /// Construct an object acquiring the contents of t
    task_t(task_t&& t)
        : id_(std::move(t.id_))
    {
    }

    /// Assign the content of t
    task_t& operator=(const task_t& t) = default;

    /// Acquire the content of t
    task_t& operator=(task_t&& t)
    {
        if (this != &t)
        {
            id_ = std::move(t.id_);
        }
        return *this;
    }

    /// Return true if the object and passed parameter have the same identifier.
    bool operator==(const task_t &t) const
    {
        return id_ == t.id_;
    }

    /// Return true if the object and passed parameter have different identifiers.
    bool operator!=(const task_t &t) const 
    {
        return !(*this == t);
    }

    /// Return true if the objects contains a valid operation identifier.
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
