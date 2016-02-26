#ifndef __CPORT_EVENT_HPP__
#define __CPORT_EVENT_HPP__

//
// event.hpp
//
// Copyright (c) 2014-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <iostream>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>

namespace cport {

namespace util {

/// This class provides mechanism to block one or more calling threads until signaled to resume.
class event {
public:

    /// Construct an event object.
    /**
     * @param signaled If this parameter is true the initial state of this event is signaled.
     *  Otherwise it is nonsignaled.
     */
    explicit event(bool signaled = false)
        : signaled_(signaled)
    {
    }

    /// Delete copy constructor.
    event(const event&) = delete;

    /// Delete assignment operator.
    event& operator=(const event&) = delete;

    /// Wait for this event to be signaled.
    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        while (!signaled_)
        {
            cond_.wait(lock);
        }
    }

    /// Block the caller thread until this event is signaled or after the specified timeout duration.
    /**
     * @param d The maximum time to wait for the event, to be signaled.
     */
    template <typename Rep, typename Period>
    std::cv_status wait_for(const std::chrono::duration<Rep, Period>& d)
    {
        std::cv_status stat = std::cv_status::no_timeout;
        std::unique_lock<std::mutex> lock(mtx_);
        while (!signaled_ && std::cv_status::timeout != stat)
        {
            stat = cond_.wait_for(lock, d);
        }
        return stat;
    }

    /// Block the caller thread until this event is signaled or until specified time point has been reached.
    /**
     * @param d The maximum time to wait for the event, to be signaled.
     */
    template <typename Clock, typename Duration>
    std::cv_status wait_until(const std::chrono::time_point<Clock, Duration>& tp)
    {
        std::cv_status stat = std::cv_status::no_timeout;
        std::unique_lock<std::mutex> lock(mtx_);
        while (!signaled_ && std::cv_status::timeout != stat)
        {
            stat = cond_.wait_until(lock, tp);
        }
        return stat;
    }

    /// Unblock one of the threads, waiting for this event to be singaled.
    void notify_one()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        signaled_ = true;
        cond_.notify_one();
    }

    /// Unblock all the threads, waiting for this event to be singaled.
    void notify_all()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        signaled_ = true;
        cond_.notify_all();
    }

    /// Reset the current state of this event to nonsignaled.
    void reset()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        signaled_ = false;
    }

private:
    std::condition_variable cond_;
    std::mutex mtx_;
    bool signaled_;
};

} // namespace util

} // namespace cport

#endif //__CPORT_EVENT_HPP__
