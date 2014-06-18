#ifndef __COMPLETION_HANDLER_HPP__
#define __COMPLETION_HANDLER_HPP__

//
// completion_handler.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cportlib/detail/completion_handler_base.hpp>
#include <cportlib/detail/obj_mem_pool.hpp>
#include <cassert>

namespace mt {

namespace detail {

template <typename Handler>
class completion_handler : public completion_handler_base {
    DECLARE_OBJ_MEMORY_POOL(completion_handler)
public:
    static completion_handler* construct(Handler h,
        std::size_t seqno, const generic_error &e)
    {
        return new completion_handler(h, seqno, e);
    }

    void invoke(const generic_error &e)
    {
        handler_(e);
    }

private:
    completion_handler(Handler handler, std::size_t seqno,
        const generic_error &e)
        : completion_handler_base(&completion_handler::invoke_,
            &completion_handler::destroy_, seqno, e)
        , handler_(handler)
    {
    }

    ~completion_handler()
    {
    }

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

IMPLEMENT_OBJ_MEMORY_POOL_T1(completion_handler, H);

} // namespace detail

} // namespace mt

#endif // __COMPLETION_HANDLER_HPP__