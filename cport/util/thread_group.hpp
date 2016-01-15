#ifndef __THREAD_GROUP_HPP__
#define __THREAD_GROUP_HPP__

//
// thread_group.hpp
//
// Copyright (c) 2014-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <algorithm>
#include <thread>
#include <vector>
#include <utility>

namespace cport {

namespace util {

class thread_group {
public:
    thread_group()
    {
    }

    template <typename Fn>
    explicit thread_group(Fn f, std::size_t count = std::thread::hardware_concurrency())
    {
        threads_.reserve(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            add(f);
        }
    }

    thread_group(const thread_group& tg) = delete;

    thread_group(thread_group&& tg)
        : threads_(std::move(tg.threads_))
    {
    }

    thread_group& operator=(const thread_group& tg) = delete;

    thread_group& operator=(thread_group&& tg)
    {
        if (this != &tg)
        {
            threads_.swap(tg.threads_);
        }
        return *this;
    }

    ~thread_group()
    {
        join();
    }

    template <typename Fn>
    void add(Fn&& f)
    {
        add(std::thread(std::forward<Fn>(f)));
    }

    void add(std::thread&& t)
    {
        threads_.push_back(std::move(t));
    }

    void join()
    {
        for (std::thread &t : threads_)
        {
            if (t.joinable())
            {
                t.join();
            }
        }

        threads_.clear();
    }

private:
    std::vector<std::thread> threads_;
};

} // namespace util

} // namespace cport

#endif //__THREAD_GROUP_HPP__


