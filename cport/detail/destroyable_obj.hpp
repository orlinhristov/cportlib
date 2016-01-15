#ifndef __DESTROYABLE_OBJ_H__
#define __DESTROYABLE_OBJ_H__

//
// destroyable_obj.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <functional>

namespace cport {
namespace detail {

class destroyable_obj {
public:
    void destroy()
    {
        destroy_helper_(this);
    }

protected:
    typedef std::function<void(destroyable_obj *)> destroy_helper_type;

    explicit destroyable_obj(destroy_helper_type helper)
        : destroy_helper_(helper)
    {
    }

    ~destroyable_obj()
    {
    }

private:
    destroy_helper_type destroy_helper_;
};

struct destroyable_deletor
{
    void operator()(destroyable_obj *obj)
    {
        obj->destroy();
    }
};

} // namespace detail

} // namespace cport

#endif // __DESTROYABLE_OBJ_H__
