#ifndef ___BASIC_TIMER_IMPL_HPP__
#define ___BASIC_TIMER_IMPL_HPP__

#include <functional>
#include <mutex>

namespace cport {

namespace timer {

namespace detail {

template <typename TimerService>
class basic_timer_impl {
public:
    using basic_timer_service = TimerService;

    using time_unit = typename TimerService::time_unit;

    using time_point = typename TimerService::time_point;

    using clock = typename TimerService::clock;

    using timer_callback = std::function<
        void(const generic_error&, const timer_id&, const time_point&)
    >;

    explicit basic_timer_impl(basic_timer_service& service);

    basic_timer_impl(const basic_timer_impl&) = delete;

    basic_timer_impl& operator=(basic_timer_impl&) = delete;

    ~basic_timer_impl();

    bool start(timer_callback callback, time_unit interval);

    bool start(timer_callback callback, time_unit interval, time_unit expire_after);

    bool start(timer_callback callback, time_unit interval, time_point expire_time);

    bool started() const;

    basic_timer_service& service() const;

    timer_id get_id() const;

    void cancel();

protected:
    template <typename Func>
    bool start(time_unit interval, timer_callback callback, time_point expire_time, Func initalizer);

private:
    // Timer service used to initialize the object
    basic_timer_service& service_;
    // Timer unique identifier
    timer_id timer_id_;
    // Set to true while the timer is in progress
    bool timer_started_;
    // Timer guard
    mutable std::mutex guard_;
};

} // namespace detail

} // namespace timer

} // namespace cport

#include <cport/timer/detail/impl/basic_timer_impl.inl>

#endif //___BASIC_TIMER_IMPL_HPP__
