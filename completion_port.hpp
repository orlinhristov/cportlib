#ifndef __COMPLETION_PORT_HPP__
#define __COMPLETION_PORT_HPP__

//
// completion_port.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <config.hpp>
#include <error_types.hpp>
#include <detail/completion_port_impl.hpp>
#include <detail/impl_accessor.hpp>

namespace mt {

/// Invokes completion handlers on callers threads. 
class completion_port {
public:

    /// Construct new completion_port
    CPORT_DECL_TYPE completion_port();

    /// Destruct the port
    /**
     * @note All ready completion handlers will be processed before destroy.
     */
    CPORT_DECL_TYPE ~completion_port();

    /// Disable copy constructor
    completion_port(const completion_port&) = delete;

    /// Disable assignment operator
    completion_port& operator=(const completion_port&) = delete;

    /// Dispatch a completion handler.
    /**
    * Post a completion handler to be invoked by any of the threads
    * blocked on wait(), wait_one(), run() and run_one() methods or
    * when methods poll() or poll_one() are called.
    * This handler will be called before any other handler posted
    * using post() method.
    *
    * @param h A completion handler to be invoked.
    *
    * @param e An error to be passed when the handler is invoked.
    */
    template <typename Handler>
    void dispatch(Handler h, const generic_error &e);

    /// Dispatch a completion handler.
    /**
    * Post a completion handler to be invoked by any of the threads
    * blocked on wait(), wait_one(), run() and run_one() methods or
    * when methods poll() or poll_one() are called.
    * This handler will be called before any other handler posted
    * using post() method.
    *
    * @param h A completion handler to be invoked.
    *
    */
    template <typename Handler>
    void dispatch(Handler h);
    
    /// Post a completion handler.
    /**
     * Post a completion handler to be invoked by any of the threads
     * blocked on wait(), wait_one(), run() and run_one() methods or
     * when methods poll() or poll_one() are called.
     *
     * @param h A completion handler to be invoked.
     *
     * @param e An error to be passed when the handler is invoked.
     */
    template <typename Handler>
    void post(Handler h, const generic_error &e);

    /// Post a completion handler.
    /**
     * Post a completion handler to be invoked by any of the threads
     * blocked on wait(), wait_one(), run() and run_one() methods or
     * when methods poll() or poll_one() are called.
     *
     * @param h The completion handler to be invoked.
     */
    template <typename Handler>
    void post(Handler h);

    /// Call a completion handler.
    /**
    * Call a completion handler in calling thread.
    *
    * @param h The completion handler to be invoked.
    *
    * @param e An error to be passed when the handler is invoked.
    */
    template <typename Handler>
    void call(Handler h, const generic_error &e);

    /// Call a completion handler.
    /**
    * Call a completion handler in calling thread
    *
    * @param h The completion handler to be invoked.
    */
    template <typename Handler>
    void call(Handler h);

    /// Wait until all enqueued operations are processed.
    /**
     * This method will block the calling thread until all enqueued operations
     * are executed and all ready completion handlers are processed.
     *
     * @returns The number of processed handlers.
     */
    CPORT_DECL_TYPE std::size_t wait();

    /// Wait for one ready completion handler to be processed.
    /**
     * This method will block the calling thread until one ready handler
     * is processed. If there is no ready completion handler and there is
     * no enqueued operation the method will return immediately.
     *
     * @returns true if a handler was processed, otherwise returns false.
     */
    CPORT_DECL_TYPE bool wait_one();

    /// Run processing of ready completion handlers.
    /**
     * This method will block the calling thread until stop() method is called.
     *
     * @returns The number of processed handlers.
     */
    CPORT_DECL_TYPE std::size_t run();

    /// Run processing of ready completion handlers.
    /**
     * This method will block the calling thread until one operation associated
     * with the port is processed or stop() method is called.
     *
     * @returns true if a handler was processed, otherwise will return false.
     */
    CPORT_DECL_TYPE bool run_one();

    /// Run processing of ready completion handlers.
    /**
     * This method will return when there is no ready handlers, without blocking
     * the calling thread.
     *
     * @returns The number of processed handlers.
     */
    CPORT_DECL_TYPE std::size_t pull();

    /// Run processing of one ready completion handlers.
    /**
     * This method will return when there is no ready handlers, without blocking
     * the calling thread.
     *
     * @returns true if a handlers was processed, otherwise will return false.
     */
    CPORT_DECL_TYPE bool pull_one();

    /// Interrupt all threads blocked on wait(), wait_one(),
    /// run() and run_one() methods.
    CPORT_DECL_TYPE void stop();

    /// Test if the port is stopped.
    CPORT_DECL_TYPE bool stopped() const;

    /// Reset stopped state of this port.
    /**
     * This method must always be called before call to wait(), wait_one(),
     * run() or run_one() methods if the port was previously stopped.
     */
    CPORT_DECL_TYPE void reset();

    /// Get the number of ready completion handlers, ready to be called.
    std::size_t ready_handlers() const;

    /// The implementation type
    typedef detail::completion_port_impl impl_type;
protected:
    CPORT_DECL_TYPE const impl_type& impl() const;
    CPORT_DECL_TYPE impl_type& impl();
private:
    template<typename T>
    friend const typename T::impl_type& detail::get_impl(const T&);

    template<typename T>
    friend typename T::impl_type& detail::get_impl(T&);
    impl_type impl_;
};

template <typename Handler>
inline void completion_port::dispatch(Handler h, const generic_error &e)
{
    impl().dispatch(h, e);
}

template <typename Handler>
inline void completion_port::dispatch(Handler h)
{
    dispatch(h, generic_error());
}

template <typename Handler>
inline void completion_port::post(Handler h, const generic_error &e)
{
    impl().post(h, impl_.next_operation_id(), e);
}

template <typename Handler>
inline void completion_port::post(Handler h)
{
    post(h, generic_error());
}

template <typename Handler>
inline void completion_port::call(Handler h, const generic_error &e)
{
    impl().call(h, e);
}

template <typename Handler>
inline void completion_port::call(Handler h)
{
    impl().call(h, generic_error());
}

} // namespace mt

#ifdef CPORT_HEADER_ONLY_LIB
#include <impl/completion_port.ipp>
#endif//CPORT_HEADER_ONLY_LIB

#endif//__COMPLETION_PORT_HPP__
