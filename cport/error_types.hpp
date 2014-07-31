#ifndef __ERROR_TYPES_HPP__
#define __ERROR_TYPES_HPP__

//
// error_types.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <stdexcept>

namespace cport {

class generic_error : public std::runtime_error {
public:
    enum { none };

    generic_error()
        : generic_error(none, std::string())
    {
    }
    generic_error(int errcode, const std::string &errmsg)
        : std::runtime_error(errmsg), code_(errcode)
    {
    }
    int code() const
    {
        return code_;
    }
    explicit operator bool() const
    {
        return none == code();
    }
private:
    int code_;
};

enum {
    operation_aborted = 0xEEEEFFFF
};

struct operation_aborted_error : generic_error {
    operation_aborted_error() 
        : generic_error(operation_aborted, "Operation aborted.")
    {
    }
};

} // namespace cport

#endif // __ERROR_TYPES_HPP__
