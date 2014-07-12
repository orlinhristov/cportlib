#ifndef __TASK_CHANNEL_GROUP_HPP__
#define __TASK_CHANNEL_GROUP_HPP__

//
// task_channel_group.hpp
//
// Copyright (c) 2014 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cportlib/task_channel.hpp>
#include <map>

namespace mt {

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

    bool erase(const key_type& key);

private:
    task_scheduler& ts_;
    std::map<key_type, task_channel::shared_ptr> groups_;
};

} // namespace mt

#include <cportlib/impl/task_channel_group.inl>

#endif //__TASK_CHANNEL_GROUP_HPP__
