#include <catch.hpp>
#include <cport/completion_port.hpp>
#include <cport/task_scheduler.hpp>
#include <cport/util/event.hpp>
#include <array>
#include <atomic>
#include <fstream>
#include <string.h>

using namespace cport;
using namespace cport::util;

TEST_CASE("By default the task scheduler runs number of workers equal to the concurrency hint", "[task_scheduler]")
{
    std::atomic<unsigned> workers{ 0u };
    const unsigned c = std::thread::hardware_concurrency();
    completion_port p;
    {
        task_scheduler ts(p, [&](task_scheduler::worker_func_prototype f) {
            ++workers;
            f();
        });

        REQUIRE(std::addressof(p) == std::addressof(ts.port()));
    }

    REQUIRE(c == workers);
}

TEST_CASE("The task scheduler starts as much workers as initialized", "[task_scheduler]")
{
    std::atomic<unsigned> workers{ 0u };
    const unsigned c = 13;
    completion_port p;
    {
        task_scheduler ts(p, c, [&](task_scheduler::worker_func_prototype f) {
            ++workers;
            f();
        });
        REQUIRE(std::addressof(p) == std::addressof(ts.port()));
    }

    REQUIRE(c == workers);
}

TEST_CASE("Scheduled task and completion handlers are called", "[task_scheduler]")
{
    completion_port p;
    task_scheduler ts(p);
    const generic_error ge(1, "Oops");

    bool taskHandlerCalled = false;
    bool compHandlerCalled = false;

    const auto& task = ts.async(
        // task handler
        [&](generic_error& e) {
            taskHandlerCalled = true;
            e = ge;
        },
        // completion handler
        [&] (const generic_error& e) {
            compHandlerCalled = true;
            REQUIRE(e.code() == ge.code());
            REQUIRE(strcmp(e.what(), ge.what()) == 0);
        }
    );

    REQUIRE(task);

    p.wait();

    REQUIRE(taskHandlerCalled);
    REQUIRE(compHandlerCalled);
}

TEST_CASE("Scheduled task can be canceled if not in processing yet", "[task_scheduler]")
{
    completion_port p;
    task_scheduler ts(p, 1);

    event e1, e2;

    task_t t1 = ts.async([&](generic_error&) {
        e2.notify_all();
        e1.wait();
    });

    task_t t2 = ts.async(
        [](generic_error&) {}
        ,
        [&](const generic_error& ge){ 
            REQUIRE(ge.code() == operation_aborted);
            e1.notify_all();
        }
    );

    e2.wait();

    REQUIRE(1 == ts.packaged_tasks());

    auto canceled = ts.cancel(t1);

    REQUIRE_FALSE(canceled);

    canceled = ts.cancel(t2);

    REQUIRE(canceled);

    p.wait();
}

TEST_CASE("All tasks that are not processing can be canceled", "[task_scheduler]")
{
    completion_port p;
    task_scheduler ts(p, 1);

    event e1, e2;

    task_t t1 = ts.async([&](generic_error&) {
        e2.notify_all();
        e1.wait();
    });

    e2.wait();

    std::array<bool, 10> callFlags;
    callFlags.fill(false);

    for (size_t i = 0; i < callFlags.size(); ++i)
    {
        ts.async(
            [&callFlags, i](generic_error&) {
                callFlags[i] = true;
            }
            ,
            [&](const generic_error& ge){
                REQUIRE(ge.code() == operation_aborted);
            }
        );
    }

    REQUIRE(callFlags.size() == ts.packaged_tasks());

    auto canceled = ts.cancel_all();

    REQUIRE(callFlags.size() == canceled);

    for (auto called : callFlags)
    {
        REQUIRE_FALSE(called);
    }

    e1.notify_all();

    p.wait();
}

TEST_CASE("A task can not be scheduled if the port is stopped", "[task_scheduler]")
{
    completion_port p;
    task_scheduler ts(p);

    p.stop();

    bool taskHandlerCalled = false;
    bool compHandlerCalled = false;

    task_t t = ts.async(
        [&](generic_error&) {
            taskHandlerCalled = true;
        },
        // completion handler
        [&](const generic_error&) {
            compHandlerCalled = true;
        }
    );

    REQUIRE_FALSE(t);

    p.wait();

    REQUIRE_FALSE(taskHandlerCalled);
    REQUIRE_FALSE(compHandlerCalled);
}

TEST_CASE("The scheduled tasks are processed simultaneously", "[task_scheduler]")
{
    std::vector<std::string> src;
    std::array<char, 1024> buf;
    std::ifstream fs(__FILE__);

    while (fs.getline(buf.data(), buf.size()))
    {
        src.emplace_back(buf.data());
    }

    event e;
    completion_port cp;
    task_scheduler ts(cp
        , std::max(2u, std::thread::hardware_concurrency())
        , [&](task_scheduler::worker_func_prototype f){
            e.wait();
            f();
    });

    std::mutex m;
    std::vector<std::string> dst;

    for (const auto& line : src)
    {
        ts.async([&](generic_error&) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            std::lock_guard<std::mutex> lock(m);
            dst.emplace_back(line);
        });
    }

    // unblock all workers
    e.notify_all();

    // wait for the completion handler to be called
    cp.wait();

    REQUIRE_FALSE(src == dst);
}
