#ifndef __COMPLETION_HANDLER_BASE_HPP__
#define __COMPLETION_HANDLER_BASE_HPP__

//
// completion_handler_base.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <cport/error_types.hpp>
#include <cport/detail/destroyable_obj.hpp>

namespace cport {

namespace detail {

class completion_handler_base : public destroyable_obj {
    typedef std::function<
        void(completion_handler_base *,
             const generic_error &)> invoke_helper_type;
public:
    completion_handler_base(invoke_helper_type invoke_helper,
                            destroy_helper_type destroy_helper,
                            std::size_t seqno,
                            const generic_error &generic_error)
        : destroyable_obj(destroy_helper)
        , invoke_helper_(invoke_helper)
        , seqno_(seqno)
        , error_(generic_error)
    {
    }

    void complete()
    {
        invoke_helper_(this, error_);
    }

    std::size_t seqno() const
    {
        return seqno_;
    }

protected:
    ~completion_handler_base()
    {
    }

private:
    invoke_helper_type invoke_helper_;
    std::size_t seqno_;
    generic_error error_;
};

} // namespace detail

} // namespace cport

#endif // __COMPLETION_HANDLER_BASE_HPP__
