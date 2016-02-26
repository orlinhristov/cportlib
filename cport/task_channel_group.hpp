#ifndef __TASK_CHANNEL_GROUP_HPP__
#define __TASK_CHANNEL_GROUP_HPP__

//
// task_channel_group.hpp
//
// Copyright (c) 2014-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/task_channel.hpp>
#include <mutex>
#include <unordered_map>

namespace cport {

/// This class provides mechanism to associate a task_cannel with a unique key.
/**
 * All task_channel objects use shared task_scheduler instance.
 *
 * @param Key Must meet the requirements of the Key template parameter type
 *  used in the standart class unordered_map.
 */
template <typename Key>
class task_channel_group {
public:
    typedef Key key_type;
    
    explicit task_channel_group(task_scheduler& ts);

    template <typename Handler>
    task_t enqueue_front(const key_type& key, Handler&& h);

    template <typename TaskHandler, typename CompletionHandler>
    task_t enqueue_front(const key_type& key, TaskHandler&& th, CompletionHandler&& ch);

    template <typename Handler>
    task_t enqueue_back(const key_type& key, Handler&& h);

    template <typename TaskHandler, typename CompletionHandler>
    task_t enqueue_back(const key_type& key, TaskHandler&& th, CompletionHandler&& ch);

    task_channel::shared_ptr get_channel(const key_type& key);

    std::size_t enqueued_tasks(const key_type& key) const;

    task_channel::shared_ptr erase(const key_type& key);

private:
    task_scheduler& ts_;
    std::unordered_map<key_type, task_channel::shared_ptr> groups_;
    std::mutex mtx_;
};

} // namespace cport

#include <cport/impl/task_channel_group.inl>

#endif //__TASK_CHANNEL_GROUP_HPP__
