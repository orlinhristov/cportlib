#ifndef __PROTECTED_T_HPP__
#define __PROTECTED_T_HPP__ 

//
// protected_t.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <utility>

namespace cport {

namespace util {

template <typename F>
struct protected_t {
    explicit protected_t(const F& f) 
        : f_(f)
    {
    }

    explicit protected_t(F&& f)
        : f_(std::move(f))
    {
    }

    template <typename... Args>
    void operator()(Args&&... args)
    {
        f_(std::forward<Args>(args)...);
    }

private:
    F f_;
};

/// Protect a bind function object from evaluation
template <typename F>
protected_t<F> protect(F&& f)
{
    return protected_t<F>(std::forward<F>(f));
}

} // namespace cport

} // namespace detail

#endif //__PROTECTED_T_HPP__
