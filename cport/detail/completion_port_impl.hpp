#ifndef __COMPLETION_PORT_IMPL_HPP__
#define __COMPLETION_PORT_IMPL_HPP__

//
// completion_port_impl.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/config.hpp>
#include <cport/detail/completion_handler_base.hpp>
#include <condition_variable>
#include <memory>
#include <queue>

namespace cport {

namespace detail {

class completion_port_impl {
    typedef std::unique_ptr<completion_handler_base, destroyable_deletor> auto_destroy;
public:
    CPORT_DECL_TYPE completion_port_impl();

    CPORT_DECL_TYPE ~completion_port_impl();

    template <typename Handler>
    void dispatch(Handler&& h, const generic_error& e);

    template <typename Handler>
    void post(Handler&& h, std::size_t seqno, const generic_error& e);

    template <typename Handler>
    void call(Handler&& h, const generic_error& e);

    CPORT_DECL_TYPE bool wait_one();

    CPORT_DECL_TYPE bool run_one();
    
    bool pull_one();

    void reset();

    void stop();

    bool stopped() const;

    std::size_t ready_handlers() const;

    std::size_t blocked_threads() const;

    CPORT_DECL_TYPE std::size_t next_operation_id();
    
private:
    CPORT_DECL_TYPE void post(completion_handler_base *h);

    CPORT_DECL_TYPE bool do_one(std::unique_lock<std::mutex> &lock);

    bool stopped_;
    // Number of threads blocked on wait_one operation
    std::size_t run_one_threads_;
    // Number of threads blocked on wait_one operation
    std::size_t wait_one_threads_;
    std::size_t queued_ops_;
    std::size_t seqno_;
    mutable std::mutex guard_;
    std::condition_variable cond_;

    struct priority_less_pred
    {
        const std::size_t gap;

        priority_less_pred& operator=(const priority_less_pred &) = delete;

        CPORT_DECL_TYPE priority_less_pred();

        CPORT_DECL_TYPE std::size_t diff(std::size_t s1, std::size_t s2);

        CPORT_DECL_TYPE bool operator()(const completion_handler_base *h1,
                                        const completion_handler_base *h2);
    };

    std::priority_queue<completion_handler_base *,
        std::vector<completion_handler_base *>, priority_less_pred> handlers_;
};

} // namespace detail

} // namespace cport

#include <cport/detail/impl/completion_port_impl.inl>
#ifdef CPORT_HEADER_ONLY_LIB
#include <cport/detail/impl/completion_port_impl.ipp>
#endif//CPORT_HEADER_ONLY_LIB

#endif // __COMPLETION_PORT_IMPL_HPP__
