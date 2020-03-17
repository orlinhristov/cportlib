#ifndef __BASIC_TIMER_SERVICE_IMPL_HPP__
#define __BASIC_TIMER_SERVICE_IMPL_HPP__

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <cport/completion_handler_wrapper.hpp>
#include <cport/error_types.hpp>
#include <cport/detail/null_handler_t.hpp>
#include <cport/timer/detail/timer_context.hpp>

/*
TODO:
 * Consider using util::event instead of directly using codition_variable to avoid
  the race condition at thread start.

 * Make sure heap related std functions are properly used
 
 */

namespace cport {

namespace timer {

namespace detail {

template <typename ClockType>
class basic_timer_service_impl 
    : public std::enable_shared_from_this<basic_timer_service_impl<ClockType>> {
public:
    using clock = ClockType;

    using time_point = typename ClockType::time_point;

    using time_unit = typename ClockType::duration;

    using timer_callback = std::function<
        void(const generic_error&, const timer_id&, const time_point&)
    >;

    using timer_context = detail::timer_context<
        time_point, time_unit, timer_callback
    >;

    explicit basic_timer_service_impl(completion_port& port);

    basic_timer_service_impl(const basic_timer_service_impl&) = delete;

    basic_timer_service_impl& operator=(const basic_timer_service_impl&) = delete;

    basic_timer_service_impl(basic_timer_service_impl&&) = delete;

    basic_timer_service_impl& operator=(basic_timer_service_impl&&) = delete;

    ~basic_timer_service_impl();

    timer_id add_timer(timer_callback callback, time_unit interval);

    timer_id add_timer(timer_callback callback, time_unit interval, time_unit expire_after);

    timer_id add_timer(timer_callback callback, time_unit interval, time_point expire_time);

    void remove_timer(timer_id id, bool notify);

private:
    void block_completion_port();

    void release_completion_port();

    void timer_thread_routine();

    template <typename Container>
    void add_timers(Container& c);

    template <typename Container>
    void del_timers(Container& c);

    template <typename Container>
    void resume_timers(Container& c,
        const time_point& current_time);

    template <typename Container>
    void check_ready_timers(Container& c,
        const time_point& current_time);

    template <typename Container>
    void cancel_timers(Container& c);

    void cancel_timer(timer_context& tc);

    void resume_timer(timer_context& tc, const time_point& current_point);

    void resume_timer(timer_id tid);

    void remove_timer_no_lock(timer_id id, bool notify);

    void invoke_callback_direct(
        timer_context& tc,
        const generic_error& e,
        const time_point& tp
    );

    void invoke_callback_indirect(
        timer_context& tc,
        const generic_error& e,
        const time_point& tp
    );

    struct del_timer_info
    {
        del_timer_info(timer_id id, bool notify)
            : id(id), notify(notify)
        {
        }

        timer_id id;

        bool notify;
    };

    std::vector<del_timer_info> del_timers_;

    std::vector<timer_context> new_timers_;

    std::vector<timer_id> resume_timers_;

    std::thread timer_thread_;

    std::condition_variable cond_var_;

    std::mutex guard_;

    completion_port& port_;

    using handler_wrapper = completion_handler_wrapper<
        cport::detail::null_handler_t
    >;
    
    handler_wrapper port_release_wrapper_;

    bool stop_thread_ = false;
};

} // namespace detail

} // namespace timer

} // namespace cport

#include <cport/timer/detail/impl/basic_timer_service_impl.inl>

#endif //__BASIC_TIMER_SERVICE_IMPL_HPP__
