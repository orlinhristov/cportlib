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

class event {
public:
    explicit event(bool signaled = false)
        : signaled_(signaled)
    {
    }

    event(const event&) = delete;

    event& operator=(const event&) = delete;

    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        while (!signaled_)
        {
            cond_.wait(lock);
        }
    }

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

    void notify_one()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        signaled_ = true;
        cond_.notify_one();
    }

    void notify_all()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        signaled_ = true;
        cond_.notify_all();
    }

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
