#include <catch.hpp>
#include <cport/completion_port.hpp>
#include <cport/task_scheduler.hpp>
#include <cport/task_channel.hpp>
#include <cport/util/event.hpp>
#include <array>
#include <atomic>
#include <fstream>
#include <string.h>

using namespace cport;
using namespace cport::util;

TEST_CASE("Enqueued task and completion handler are called", "[task_channel]")
{
    completion_port p;
    task_scheduler ts(p);
    task_channel::shared_ptr tc = task_channel::make_shared(ts);

    bool taskHandlerCalled = false;
    bool compHandlerCalled = false;

    const auto& task = tc->enqueue_back(
        // task handler
        [&](generic_error& e) {
            taskHandlerCalled = true;
        },
        // completion handler
        [&](const generic_error& e) {
            compHandlerCalled = true;
        }
    );

    REQUIRE(task);

    p.wait();

    REQUIRE(taskHandlerCalled);
    REQUIRE(compHandlerCalled);
}

TEST_CASE("The task_channel::current_task() method will return the identifier of the task that is currently executing", "[task_channel]")
{
    completion_port p;
    task_scheduler ts(p);
    task_channel::shared_ptr tc = task_channel::make_shared(ts);

    const auto& task = tc->enqueue_back(
        // task handler
        [&](generic_error& e) {
        },
        // completion handler
        [&](const generic_error& e) {
        }
    );

    REQUIRE(task);
    REQUIRE(task == tc->current_task());

    p.wait();
}

TEST_CASE("Enqueuing in front of the channel takes priority over enqueuing in back", "[task_channel]")
{
    completion_port p;
    task_scheduler ts(p);
    task_channel::shared_ptr tc = task_channel::make_shared(ts);

    // This task will delay execution of the next handlers
    // until completion_port::wait() is called.
    tc->enqueue_back([&](const generic_error& e){
    });

    bool firstHandlerCalled  = false;
    bool secondHandlerCalled = false;

    tc->enqueue_back([&](const generic_error& e){
        firstHandlerCalled = true;
        REQUIRE(secondHandlerCalled);
    });

    tc->enqueue_front([&](const generic_error& e){
        secondHandlerCalled = true;
        REQUIRE_FALSE(firstHandlerCalled);
    });

    REQUIRE(2 == tc->enqueued_tasks());

    p.wait();

    REQUIRE(firstHandlerCalled);
    REQUIRE(secondHandlerCalled);
}

TEST_CASE("Enqueued task can be canceled if not in processing yet", "[task_channel]")
{
    completion_port p;
    task_scheduler ts(p, 1);
    task_channel::shared_ptr tc = task_channel::make_shared(ts);

    // This task will delay execution of the next handlers
    // until completion_port::wait() is called.
    task_t t1 = tc->enqueue_back([&](const generic_error& e){
    });

    task_t t2 = tc->enqueue_back(
        [](generic_error&) {}
        ,
        [&](const generic_error& ge){ 
            REQUIRE(ge.code() == operation_aborted);
        }
    );

    REQUIRE(1 == tc->enqueued_tasks());

    auto canceled = tc->cancel(t1);

    REQUIRE_FALSE(canceled);

    canceled = tc->cancel(t2);

    REQUIRE(canceled);

    p.wait();
}

TEST_CASE("All enqueued tasks that are not processing can be canceled", "[task_channel]")
{
    completion_port p;
    task_scheduler ts(p, 1);
    task_channel::shared_ptr tc = task_channel::make_shared(ts);

    // This task will delay execution of the next handlers
    // until completion_port::wait() is called.
    task_t t1 = tc->enqueue_back([&](const generic_error& e){
    });

    std::array<bool, 10> callFlags;
    callFlags.assign(false);

    for (size_t i = 0; i < callFlags.size(); ++i)
    {
        tc->enqueue_back(
            [&callFlags, i](generic_error&) {
                callFlags[i] = true;
            }
            ,
            [&](const generic_error& ge){
                REQUIRE(ge.code() == operation_aborted);
            }
        );
    }

    REQUIRE(callFlags.size() == tc->enqueued_tasks());

    auto canceled = tc->cancel_all();

    REQUIRE(callFlags.size() == canceled);

    for (auto called : callFlags)
    {
        REQUIRE_FALSE(called);
    }

    p.wait();
}

TEST_CASE("Task channel guarantees FIFO principe", "[task_channel]")
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

    auto tc = task_channel::make_shared(ts);
    for (const auto& line : src)
    {
        tc->enqueue_back([&](generic_error&) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            std::lock_guard<std::mutex> lock(m);
            dst.emplace_back(line);
        });
    }

    // unblock all workers
    e.notify_all();

    // wait for the completion handler to be called
    cp.wait();

    REQUIRE(src == dst);
}