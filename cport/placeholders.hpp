#ifndef __MT_PLACEHOLDERS_HPP__
#define __MT_PLACEHOLDERS_HPP__

//
// placeholders.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <functional>

namespace cport {
namespace placeholders {
    /// Used to specify error placeholder in calls to standard bind function.
    const decltype(std::placeholders::_1) error = std::placeholders::_1;
}
}

#endif//__MT_PLACEHOLDERS_HPP__
