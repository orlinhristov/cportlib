#ifndef __IMPL_ACCESSOR_HPP__
#define __IMPL_ACCESSOR_HPP__

//
// impl_accessor.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

namespace cport {
namespace detail {
    template <typename T>
    inline const typename T::impl_type& get_impl(const T &t)
    {
        return t.impl();
    }

    template <typename T>
    inline typename T::impl_type& get_impl(T &t)
    {
        return t.impl();
    }
}
}

#endif //__IMPL_ACCESSOR_HPP__
