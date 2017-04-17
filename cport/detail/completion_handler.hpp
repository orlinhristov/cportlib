#ifndef __COMPLETION_HANDLER_HPP__
#define __COMPLETION_HANDLER_HPP__

//
// completion_handler.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/detail/completion_handler_base.hpp>
#include <cport/detail/obj_mem_pool.hpp>
#include <cassert>

namespace cport {

namespace detail {

template <typename Handler>
class completion_handler : public completion_handler_base {
    DECLARE_OBJ_MEMORY_POOL(completion_handler)
public:
    completion_handler(const Handler& handler, std::size_t seqno,
        const generic_error& e)
        : completion_handler_base(&completion_handler::invoke_,
            &completion_handler::destroy_, seqno, e)
        , handler_(handler)
    {
    }

    completion_handler(Handler&& handler, std::size_t seqno,
        const generic_error& e)
        : completion_handler_base(&completion_handler::invoke_,
            &completion_handler::destroy_, seqno, e)
        , handler_(std::move(handler))
    {
    }

    ~completion_handler()
    {
    }

    void invoke(const generic_error &e)
    {
        handler_(e);
    }

private:
    typedef completion_handler<Handler> this_type;

    static void destroy_(destroyable_obj *base)
    {
        assert(base != nullptr);
        delete static_cast<this_type *>(base);
    }

    static void invoke_(completion_handler_base *base,
        const generic_error &e)
    {
        assert(base != nullptr);
        static_cast<this_type *>(base)->invoke(e);
    }

    Handler handler_;
};

template <typename Handler>
inline completion_handler<typename std::remove_reference<Handler>::type>* 
create_completion_handler(Handler&& h, std::size_t seq, const generic_error& e)
{
    return new completion_handler<typename std::remove_reference<Handler>::type>(
        std::forward<typename std::remove_reference<Handler>::type>(h), seq, e);
}

IMPLEMENT_OBJ_MEMORY_POOL_T1(completion_handler, H);

} // namespace detail

} // namespace cport

#endif // __COMPLETION_HANDLER_HPP__
