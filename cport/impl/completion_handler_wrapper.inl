#ifndef __COMPLETION_HANDLER_WRAPPER_INL__
#define __COMPLETION_HANDLER_WRAPPER_INL__

//
// completion_handler_wrapper.inl
//
// Copyright (c) 2017 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/completion_port.hpp>
#include <cport/error_types.hpp>
#include <cport/detail/completion_port_impl.hpp>
#include <cport/detail/impl_accessor.hpp>
#include <cport/detail/null_handler_t.hpp>
#include <algorithm>

namespace cport {

/// Create a handler wrapper, associated with the completion_port instance.
/**
 * Enforce completion port to wait for the wrapped handler to be posted.
 *
 * @param h A completion handler to be invoked.
 *
 * @param p A port to associated with.
 *
 * @returns wrapped completion handler.
 */
template <typename Handler>
inline completion_handler_wrapper<Handler> wrap_completion_handler(Handler&& h, completion_port& p)
{
    return completion_handler_wrapper<Handler>(std::forward<Handler>(h), p);
}

template <typename Handler>
inline completion_handler_wrapper<Handler>::completion_handler_wrapper(Handler&& handler, completion_port& port)
    : port_impl_(&detail::get_impl(port))
    , op_id_(port_impl_->next_operation_id())
    , handler_(std::forward<Handler>(handler))
{
}

template <typename Handler>
inline completion_handler_wrapper<Handler>::completion_handler_wrapper(completion_handler_wrapper&& rhs)
    : port_impl_(rhs.port_impl_)
    , op_id_(std::move(rhs.op_id_))
    , handler_(std::forward<Handler>(rhs.handler_))
{
}

template <typename Handler>
inline auto completion_handler_wrapper<Handler>::operator=(completion_handler_wrapper<Handler>&& rhs) -> completion_handler_wrapper<Handler>&
{
    if (&rhs != this)
    {
        rhs.swap(*this);
    }
    return *this;
}

template <typename Handler>
inline completion_handler_wrapper<Handler>::~completion_handler_wrapper()
{
    if (op_id_.valid())
    {
        const generic_error e;
        port_impl_->post(detail::null_handler_t(), op_id_, e);
    }
}

template <typename Handler>
inline void completion_handler_wrapper<Handler>::swap(completion_handler_wrapper& rhs)
{
    using std::swap;
    swap(port_impl_, rhs.port_impl_);
    swap(op_id_, rhs.op_id_);
    swap(handler_, rhs.handler_);
}

template <typename Handler>
inline void completion_handler_wrapper<Handler>::operator()()
{
    const generic_error e;
    operator()(e);
}

template <typename Handler>
inline void completion_handler_wrapper<Handler>::operator()(const generic_error& e)
{
    if (op_id_.valid())
    {
        detail::operation_id op_id;
        std::swap(op_id_, op_id);
        port_impl_->post(handler_, op_id, e);
    }
    else
    {
        throw std::bad_function_call();
    }
}

} // namespace cport

#endif //__COMPLETION_HANDLER_WRAPPER_INL__
