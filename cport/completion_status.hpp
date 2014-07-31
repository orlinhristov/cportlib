#ifndef __COMPLETION_STATE_HPP__
#define __COMPLETION_STATE_HPP__

//
// completion_handler_base.hpp
//
// Copyright (c) 2013-2014 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

namespace cport {

// The scoped enumeration describes current task execution status

enum class completion_status { 
    none,
    scheduled, // The operation is scheduled for execution.
    executing, // The operation is currently executing.
    canceled, // The operation execution is aborted.
    complete, // The operation is executed.
};

}

#endif//__COMPLETION_STATE_HPP__
