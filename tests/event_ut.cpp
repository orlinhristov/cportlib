#include <catch.hpp>
#include <cport/util/event.hpp>
#include <cport/util/thread_group.hpp>
#include <algorithm>
#include <atomic>

using namespace cport::util;

TEST_CASE("Block current thread on wait_for(), until timed out")
{
    event e;

    REQUIRE_FALSE(e.signaled());

    const auto result = e.wait_for(std::chrono::nanoseconds(1));

    REQUIRE(std::cv_status::timeout == result);

    REQUIRE_FALSE(e.signaled());
}

TEST_CASE("Block current thread on wait_until(), until timed out")
{
    event e;

    REQUIRE_FALSE(e.signaled());

    const auto result = e.wait_until(std::chrono::system_clock::now());

    REQUIRE(std::cv_status::timeout == result);

    REQUIRE_FALSE(e.signaled());
}

TEST_CASE("Block current thread on wait(), until signaled")
{
    event e;

    REQUIRE_FALSE(e.signaled());

    std::thread t([&](){ e.notify_all(); });

    e.wait();

    t.join();

    REQUIRE(e.signaled());

    e.reset();

    REQUIRE_FALSE(e.signaled());
}

TEST_CASE("Block current thread on wait_for(), until signaled")
{
    event e;

    REQUIRE_FALSE(e.signaled());

    std::thread t([&](){ e.notify_all(); });

    const auto result = e.wait_for(std::chrono::hours(1));

    t.join();

    REQUIRE(e.signaled());

    e.reset();

    REQUIRE_FALSE(e.signaled());

    REQUIRE(std::cv_status::no_timeout == result);
}

TEST_CASE("Block current thread on wait_until(), until signaled")
{
    event e;

    REQUIRE_FALSE(e.signaled());

    std::thread t([&](){ e.notify_all(); });

    const auto result = e.wait_until(std::chrono::time_point<std::chrono::system_clock>::max());

    t.join();

    REQUIRE(e.signaled());

    REQUIRE(std::cv_status::no_timeout == result);

    e.reset();

    REQUIRE_FALSE(e.signaled());
}

TEST_CASE("Block multiple threads on event until all signaled")
{
    event e;

    REQUIRE_FALSE(e.signaled());
    
    const auto concurrency = std::max(2u, std::thread::hardware_concurrency());

    std::atomic<unsigned> count{ 0 };

    thread_group tg([&](){
        const auto result = e.wait_for(std::chrono::seconds(5));
        if (std::cv_status::timeout != result)
            ++count;
    }, concurrency);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    e.notify_all();

    tg.join();

    REQUIRE(e.signaled());

    e.reset();

    REQUIRE_FALSE(e.signaled());

    REQUIRE(concurrency == count);
}

TEST_CASE("Block multiple threads on event until one is signaled")
{
    event e;

    REQUIRE_FALSE(e.signaled());

    const auto concurrency = std::max(2u, std::thread::hardware_concurrency());

    std::atomic<unsigned> count{ 0 };

    thread_group tg([&](){
        const auto result = e.wait_for(std::chrono::seconds(5));
        if (std::cv_status::timeout != result)
            ++count;
    }, concurrency);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    e.notify_one();

    tg.join();

    REQUIRE(e.signaled());

    e.reset();

    REQUIRE_FALSE(e.signaled());

    REQUIRE(1 == count);
}
