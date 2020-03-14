#ifndef __TIMER_ERROR_TYPES_HPP__
#define __TIMER_ERROR_TYPES_HPP__

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

namespace timer {

enum {
    /// Used to report that a timer has expired. 
    timer_expired = 0xEEEEFFFE,
    invalid_timer = 0xEEEEFFFE
};

/// Defines a type used to report that a tiemr has expired. 
struct timer_expired_error : generic_error {
    timer_expired_error()
        : generic_error(timer_expired, "Timer expired.")
    {
    }
};

struct invalid_timer_error : generic_error
{
    invalid_timer_error()
        : generic_error(invalid_timer, "Invalid timer.")
    {
    }
};

} // namespace timer

} // namespace cport

#endif // __TIMER_ERROR_TYPES_HPP__
