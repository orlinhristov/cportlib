#ifndef __TASK_CHANNEL_GROUP_INL__
#define __TASK_CHANNEL_GROUP_INL__

//
// task_channel_group.inl
//
// Copyright (c) 2014-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

namespace cport {

template <typename Key>
inline task_channel_group<Key>::task_channel_group(task_scheduler& ts)
    : ts_(ts)
{
}

template <typename Key>
template <typename Handler>
inline task_t task_channel_group<Key>::enqueue_front(const key_type& key, Handler&& h)
{
    return get_channel(key)->enqueue_front(std::forward<Handler>(h));
}

template <typename Key>
template <typename TaskHandler, typename CompletionHandler>
inline task_t task_channel_group<Key>::enqueue_front(const key_type& key, TaskHandler&& th, CompletionHandler&& ch)
{
    return get_channel(key)->enqueue_front(std::forward<TaskHandler>(th), std::forward<CompletionHandler>(ch));
}

template <typename Key>
template <typename Handler>
inline task_t task_channel_group<Key>::enqueue_back(const key_type& key, Handler&& h)
{
    return get_channel(key)->enqueue_back(std::forward<Handler>(h));
}

template <typename Key>
template <typename TaskHandler, typename CompletionHandler>
inline task_t task_channel_group<Key>::enqueue_back(const key_type& key, TaskHandler&& th, CompletionHandler&& ch)
{
    return get_channel(key)->enqueue_back(std::forward<TaskHandler>(th), std::forward<CompletionHandler>(ch));
}

template <typename Key>
inline task_channel::shared_ptr task_channel_group<Key>::get_channel(const key_type &key)
{
    std::unique_lock<std::mutex> lock(mtx_);

    auto i = groups_.find(key);

    if (i == groups_.end())
    {
        i = groups_.emplace(key, task_channel::make_shared(ts_)).first;
    }

    return i->second;
}

template <typename Key>
inline std::size_t task_channel_group<Key>::enqueued_tasks(const key_type& key) const
{
    return get_channel(key)->enqueued_tasks();
}

template <typename Key>
inline task_channel::shared_ptr task_channel_group<Key>::erase(const key_type& key)
{
    task_channel::shared_ptr channel;

    std::unique_lock<std::mutex> lock(mtx_);

    auto i = groups_.find(key);

    if (i != groups_.end())
    {
        channel.swap(i->second);

        groups_.erase(i);
    }

    return channel;
}

} // namespace cport

#endif //__TASK_CHANNEL_GROUP_INL__
