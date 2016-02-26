#ifndef __ERROR_TYPES_HPP__
#define __ERROR_TYPES_HPP__

//
// error_types.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <stdexcept>

namespace cport {

/// A generic error type
/**
 * It reports errors related to the library and such that are beyond
 *  the library scope (user-defined).
 */
class generic_error : public std::runtime_error {
public:
    enum { none };

    /// Construct an objects that represents no error.
    generic_error()
        : generic_error(none, std::string())
    {
    }

    /// Construct an object with specific error code and error message.
    generic_error(int errcode, const std::string &errmsg)
        : std::runtime_error(errmsg), code_(errcode)
    {
    }

    /// Return error code.
    int code() const
    {
        return code_;
    }

    /// Return true if the object represents an error.
    explicit operator bool() const
    {
        return none != code();
    }
private:
    int code_;
};

enum {
    /// Used to report that an operation was aborted. 
    operation_aborted = 0xEEEEFFFF
};

/// Defines a type used to report that an operation was aborted. 
struct operation_aborted_error : generic_error {
    operation_aborted_error() 
        : generic_error(operation_aborted, "Operation aborted.")
    {
    }
};

} // namespace cport

#endif // __ERROR_TYPES_HPP__
