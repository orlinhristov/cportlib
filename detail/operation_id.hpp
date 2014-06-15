#ifndef __OPERATION_HANDLER_T_HPP__
#define __OPERATION_HANDLER_T_HPP__

//
// operation_id.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cstddef>
#ifdef CPORT_ENABLE_TASK_STATE
#include <detail/event.hpp>
#include <atomic>
#include <memory>
#include <utility>
#endif

namespace mt {

namespace detail {

class operation_id {
public:
    typedef std::size_t value_type;

    operation_id(value_type vt = value_type())
        : value_(vt)
#ifdef CPORT_ENABLE_TASK_STATE
        , state_(new atomic_state(none))
        , state_event_(new event())
#endif
    {
    }

    operation_id(const operation_id & op) = default;

    operation_id(operation_id&& op)
        : value_(op.value_)
#ifdef CPORT_ENABLE_TASK_STATE
        , state_(std::move(op.state_))
        , state_event_(std::move(op.state_event_))
#endif
    {
    }

    operation_id& operator=(const operation_id & op) = default;

    operation_id& operator=(operation_id&& op)
    {
        if (this != &op)
        {
            value_ = op.value_;
#ifdef CPORT_ENABLE_TASK_STATE
            state_ = std::move(op.state_);
            state_event_ = std::move(op.state_event_);
#endif
        }
        return *this;
    }

    bool operator==(const operation_id &op) const
    {
        return value_ == op.value_;
    }

    bool operator!=(const operation_id &op) const
    {
        return !(*this == op);
    }

    bool operator<(const operation_id &op) const
    {
        return value_ < op.value_;
    }

    bool valid() const
    {
        return value_ != value_type();
    }

    operator value_type() const
    {
        return value_;
    }
#ifdef CPORT_ENABLE_TASK_STATE
    enum state { none, scheduled, executing, canceled, complete };

    state get_state() const
    {
        return state_->load();
    }

    void set_state(state s)
    {
        state_->store(s);
        if (canceled == s || complete == s)
            state_event_->notify_all();
    }

    void wait()
    {
        state_event_->wait();
    }
#endif

private:

#ifdef CPORT_ENABLE_TASK_STATE
    typedef std::atomic<state> atomic_state;
    typedef std::shared_ptr<atomic_state> shared_atomic_state;
    shared_atomic_state state_;

    typedef std::shared_ptr<event> shared_event;
    shared_event state_event_;
#endif
    value_type value_;
};

} // namespace detail

} // namespace mt

#endif // __OPERATION_HANDLER_T_HPP__