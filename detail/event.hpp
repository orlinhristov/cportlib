#ifndef __CPORT_EVENT_HPP__
#define __CPORT_EVENT_HPP__

#include <iostream>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>

namespace mt {

namespace detail {

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

} // namespace detail

} // namespace mt

#endif //__CPORT_EVENT_HPP__