#ifndef __COMPLETION_HANDLER_WRAPPER_HPP__
#define __COMPLETION_HANDLER_WRAPPER_HPP__

//
// completion_handler_wrapper.hpp
//
// Copyright (c) 2017 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/detail/operation_id.hpp>

namespace cport {

class completion_port;
class generic_error;

namespace detail {
    class completion_port_impl;
}

/// This class wraps a callable object associated with a completion_port
/**
 * The handler wrapper creates a relationship with the port, which will enforce
 *  it to block on wait() or wait_one() until one of the two functors methods is called.
 *
 * The completion_handler_wrapper is not copy constructable or copy assignable,
 *  although it is move constructable and move assignable.
 */
template <typename Handler>
class completion_handler_wrapper {
public:
    /// Construct a completion handler wrapper.
    /**
     *
     * @param handler A completion handler to be wrapped.
     *
     * @param port A completion port which will be used to invoke the handler.
     */
    completion_handler_wrapper(Handler&& handler, completion_port& port);

    /// Delete copy constructor.
    completion_handler_wrapper(const completion_handler_wrapper&) = delete;

    /// Delete copy constructor.
    completion_handler_wrapper& operator=(const completion_handler_wrapper& rsh) = delete;

    /// Construct a wrapper to represent the handler that was represented by rhs
    completion_handler_wrapper(completion_handler_wrapper&& rhs);

    /// Move the ownership of the handler represented by rhs to this wrapper 
    completion_handler_wrapper& operator=(completion_handler_wrapper&& rhs);
    
    /// Destruct the wrapper
    ~completion_handler_wrapper();

    /// Swap handlers represented by this wrapper and rhs
    void swap(completion_handler_wrapper& rhs);

    /// Invoke the handler stored in this wrapper.
    /**
     * @throw std::bad_function_call if the method is called more than once.
     */
    void operator()();

    /// Invoke the handler stored in this wrapper with parameter e
    /**
     * @param e An error object to be passed to the handler.
     *
     * @throw std::bad_function_call if the method is called more than once.
     */
    void operator()(const generic_error& e);

private:
    detail::completion_port_impl* port_impl_;
    detail::operation_id op_id_;
    Handler handler_;
};

} // namespace cport

#include <cport/impl/completion_handler_wrapper.inl>

#endif //__COMPLETION_HANDLER_WRAPPER_HPP__
