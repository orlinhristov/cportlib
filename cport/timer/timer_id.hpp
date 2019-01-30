#ifndef TIMER_ID_HPP_INCLUDED
#define TIMER_ID_HPP_INCLUDED

#include <atomic>
#include <limits>

namespace cport
{

namespace timer
{

using timer_id = std::size_t;

constexpr timer_id invalid_timer_id = timer_id{};

inline timer_id get_next_timer_id()
{
    static std::atomic<timer_id> next = timer_id{};

    if (next == std::numeric_limits<timer_id>::max())
    {
        next = timer_id{};
    }

    return ++next;
}

} // namespace timer

} // namespace cport

#endif //TIMER_ID_HPP_INCLUDED