#ifndef __COMPLETION_STATE_HPP__
#define __COMPLETION_STATE_HPP__

//
// completion_handler_base.hpp
//
// Copyright (c) 2013 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

enum class completion_status : unsigned short
{ 
    none,
    scheduled,
    executing,
    canceled,
    complete,
};

#endif//__COMPLETION_STATE_HPP__