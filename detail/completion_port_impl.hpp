#ifndef __COMPLETION_PORT_IMPL_HPP__
#define __COMPLETION_PORT_IMPL_HPP__

//
// completion_port_impl.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <config.hpp>
#include <detail/completion_handler.hpp>
#include <detail/null_handler_t.hpp>
#include <cassert>
#include <condition_variable>
#include <memory>
#include <queue>

namespace mt {

namespace detail {

class completion_port_impl {
    typedef std::unique_ptr<completion_handler_base, destroyable_deletor> auto_destroy;
public:
    CPORT_DECL_TYPE completion_port_impl();

	CPORT_DECL_TYPE ~completion_port_impl();

    template <typename Handler>
    void dispatch(Handler h, const generic_error &e);

    template <typename Handler>
    void post(Handler h, std::size_t seqno, const generic_error &e);

    template <typename Handler>
    void call(Handler h, const generic_error &e);

	CPORT_DECL_TYPE bool wait_one();

	CPORT_DECL_TYPE bool run_one();
    
	CPORT_DECL_TYPE bool pull_one();

	CPORT_DECL_TYPE void reset();

	CPORT_DECL_TYPE void stop();

	CPORT_DECL_TYPE bool stopped() const;

	CPORT_DECL_TYPE std::size_t ready_handlers() const;

	CPORT_DECL_TYPE std::size_t next_operation_id();


private:
	CPORT_DECL_TYPE bool do_one(std::unique_lock<std::mutex> &lock);

    bool stopped_;
    // Number of threads blocked on wait_one operation
    std::size_t wait_one_threads_;
    std::size_t queued_ops_;
    std::size_t seqno_;
    mutable std::mutex guard_;
    std::condition_variable cond_;

    struct priority_less_pred
    {
        const std::size_t gap;

        priority_less_pred& operator=(const priority_less_pred &);

        priority_less_pred()
            : gap(std::numeric_limits<std::size_t>::max()/2)
        {
        }

        std::size_t diff(std::size_t s1, std::size_t s2)
        {
            return s1 > s2 ? s1 - s2 : s2 - s1;
        }

        bool operator()(const completion_handler_base *h1,
                        const completion_handler_base *h2)
        {
            if (h1->seqno() == 0)
                return false;

            if (h2->seqno() == 0)
                return true;

            if (diff(h1->seqno(), h2->seqno()) > gap)
                return h1->seqno() < h2->seqno();
            return h1->seqno() > h2->seqno();
        }
    };

    std::priority_queue<completion_handler_base *,
        std::vector<completion_handler_base *>, priority_less_pred> handlers_;
};

template <typename Handler>
void completion_port_impl::post(Handler h,
                                std::size_t seqno,
                                const generic_error &e)
{
    completion_handler_base *wrapper = 
        completion_handler<Handler>::construct(h, seqno, e);

    std::unique_lock<std::mutex> lock(guard_);
    handlers_.push(wrapper);

    assert(seqno == 0 || queued_ops_ > 0);
    if (seqno > 0)
        --queued_ops_;

    if (queued_ops_ == 0 && wait_one_threads_ > 0)
        cond_.notify_all();
    else
        cond_.notify_one();
}

template <typename Handler>
inline void completion_port_impl::dispatch(Handler h, const generic_error &e)
{
    post(h, 0, e);
}

template <typename Handler>
inline void completion_port_impl::call(Handler h, const generic_error &e)
{
    h(e);
}

} // namespace detail

} // namespace mt

#ifdef CPORT_HEADER_ONLY_LIB
#include <detail/impl/completion_port_impl.ipp>
#endif//CPORT_HEADER_ONLY_LIB

#endif // __COMPLETION_PORT_IMPL_HPP__