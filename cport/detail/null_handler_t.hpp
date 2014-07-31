#ifndef __NULL_HANDLER_TYPE_HPP__
#define __NULL_HANDLER_TYPE_HPP__

//
// null_handler_t.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

namespace cport {

class generic_error;

namespace detail {

struct null_handler_t
{
    null_handler_t()
    {
    }
    void operator()(const generic_error &) const
    {
    }
};

} // namespace detail

} // namespace cport

#endif
