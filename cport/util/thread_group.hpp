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

/// This class provides a mechanism to manage the lifecycle of a group of threads.
/**
 * Defines an interface similar to the one, that the standart library defines,
 *  for a singe thread of execution.
 */
class thread_group {
public:

    /// Construct an empty group.
    thread_group()
    {
    }

    /// Construct a group of threads.
    /**
     * @param f A callable object to be run in all threads created by the constructor.
     *
     * @param count The number of threads to be created. If this parameter is not specified,
     *  the object will starta a number of threads, equal to concurrent threads,
     *  supported by the system.
     */
    template <typename Fn>
    explicit thread_group(Fn f, std::size_t count = std::thread::hardware_concurrency())
    {
        threads_.reserve(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            add(f);
        }
    }

    /// Delete copy constructor.
    thread_group(const thread_group& tg) = delete;

    /// Construct an object acquiring the threads running in tg.
    thread_group(thread_group&& tg)
        : threads_(std::move(tg.threads_))
    {
    }

    /// Delete assigment operator.
    thread_group& operator=(const thread_group& tg) = delete;

    /// Acquire the threads running in tg.
    thread_group& operator=(thread_group&& tg)
    {
        if (this != &tg)
        {
            threads_.swap(tg.threads_);
        }
        return *this;
    }

    /// Destruct the group.
    /**
     * Will wait for all threads in the group to finish.
     */
    ~thread_group()
    {
        join();
    }

    /// Add new thread to the group.
    /**
     * @param f A callable object to be run in the newly created thread.
     */
    template <typename Fn>
    void add(Fn&& f)
    {
        add(std::thread(std::forward<Fn>(f)));
    }

    /// Add an existing thread to the group.
    /**
     * @param t The thread to be added.
     */
    void add(std::thread&& t)
    {
        threads_.push_back(std::move(t));
    }

    /// Wait for all running threads in this group to finish.
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


