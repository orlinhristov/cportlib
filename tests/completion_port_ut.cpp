#include <catch.hpp>
#include <cport/completion_port.hpp>
#include <cport/util/event.hpp>
#include <algorithm>

using namespace cport;
using namespace cport::util;

TEST_CASE("completion_port", "cportlib")
{
    completion_port cp;

    REQUIRE(0 == cp.ready_handlers());

    SECTION("wait() method returns immediately if there is no ready completion handlers")
    {
        const auto count = cp.wait();
        REQUIRE(0 == count);
    }

    SECTION("wait_one() method returns immediately if there is no ready completion handlers")
    {
        const auto result = cp.wait_one();
        REQUIRE_FALSE(result);
    }

    SECTION("pull() method returns immediately if there is no ready completion handlers")
    {
        const auto count = cp.pull();
        REQUIRE(0 == count);
    }

    SECTION("pull_one() method returns immediately if there is no ready completion handlers")
    {
        const auto result = cp.pull_one();
        REQUIRE_FALSE(result);
    }

    SECTION("Completion handlers were called")
    {
        cp.call([](const generic_error& e){
            // completion handler must be called with no error
            REQUIRE_FALSE(e);
        });

        cp.call([](const generic_error& e){
            // completion handler must be called with no error
            REQUIRE(e.code() == operation_aborted);
        }, operation_aborted_error());

        REQUIRE(0 == cp.ready_handlers());

        SECTION("wait() method returns immediately if there is no ready completion handlers")
        {
            const auto count = cp.wait();
            REQUIRE(0 == count);
        }

        SECTION("wait_one() method returns immediately if there is no ready completion handlers")
        {
            const auto result = cp.wait_one();
            REQUIRE_FALSE(result);
        }

        SECTION("pull() method returns immediately if there is no ready completion handlers")
        {
            const auto count = cp.pull();
            REQUIRE(0 == count);
        }

        SECTION("pull_one() method returns immediately if there is no ready completion handlers")
        {
            const auto result = cp.pull_one();
            REQUIRE_FALSE(result);
        }

        REQUIRE(0 == cp.ready_handlers());
    }

    SECTION("Completion handlers were posted")
    {
        const auto tid = std::this_thread::get_id();

        cp.post([&](const generic_error& e){
            // completion handler must be called with no error
            REQUIRE_FALSE(e);
            REQUIRE_FALSE(tid == std::this_thread::get_id());
        });

        cp.post([&](const generic_error& e){
            // completion handler must be called with no error
            REQUIRE(e.code() == operation_aborted);
            REQUIRE_FALSE(tid == std::this_thread::get_id());
        }, operation_aborted_error());

        REQUIRE(2 == cp.ready_handlers());

        SECTION("wait() method blocks until all ready completion handlers are invoked")
        {
            std::thread([&](){
                const auto count = cp.wait();
                REQUIRE(2 == count);
            }).join();
        }

        SECTION("wait_one() method blocks until one ready completion handler is invoked")
        {
            std::thread([&](){
                auto count = 0;
                while (cp.wait_one())
                    ++count;
                REQUIRE(2 == count);
            }).join();
        }

        SECTION("pull() method blocks until all ready completion handlers are invoked")
        {
            std::thread([&](){
                const auto count = cp.pull();
                REQUIRE(2 == count);
            }).join();
        }

        SECTION("pull_one() method blocks until one ready completion handler is invoked")
        {
            std::thread([&](){
                auto count = 0;
                while (cp.pull_one())
                    ++count;
                REQUIRE(2 == count);
            }).join();
        }

        REQUIRE(0 == cp.ready_handlers());
    }

    SECTION("Completion handlers were dispatched")
    {
        const auto tid = std::this_thread::get_id();

        cp.dispatch([&](const generic_error& e){
            // completion handler must be called with no error
            REQUIRE_FALSE(e);
            REQUIRE_FALSE(tid == std::this_thread::get_id());
        });

        cp.dispatch([&](const generic_error& e){
            // completion handler must be called with no error
            REQUIRE(e.code() == operation_aborted);
            REQUIRE_FALSE(tid == std::this_thread::get_id());
        }, operation_aborted_error());

        REQUIRE(2 == cp.ready_handlers());

        SECTION("wait() method blocks until all ready completion handlers are invoked")
        {
            std::thread([&](){
                const auto count = cp.wait();
                REQUIRE(2 == count);
            }).join();
        }

        SECTION("wait_one() method blocks until one ready completion handler is invoked")
        {
            std::thread([&](){
                auto count = 0;
                while (cp.wait_one())
                    ++count;
                REQUIRE(2 == count);
            }).join();
        }

        SECTION("pull() method blocks until all ready completion handlers are invoked")
        {
            std::thread([&](){
                const auto count = cp.pull();
                REQUIRE(2 == count);
            }).join();
        }

        SECTION("pull_one() method blocks until one ready completion handler is invoked")
        {
            std::thread([&](){
                auto count = 0;
                while (cp.pull_one())
                    ++count;
                REQUIRE(2 == count);
            }).join();
        }

        REQUIRE(0 == cp.ready_handlers());
    }

    SECTION("Dispatched handler takes priority over posted one")
    {
        bool dispHandlerCalled = false;
        bool postHandlerCalled = false;

        cp.post([&](const generic_error& e){
            postHandlerCalled = true;
            REQUIRE(dispHandlerCalled);
        });

        cp.dispatch([&](const generic_error& e){
            dispHandlerCalled = true;
            REQUIRE_FALSE(postHandlerCalled);
        });

        REQUIRE(2 == cp.ready_handlers());

        cp.wait();

        REQUIRE(0 == cp.ready_handlers());

        REQUIRE(dispHandlerCalled);
        REQUIRE(postHandlerCalled);
    }

    SECTION("The run_one() method blocks the calling thread until stop is called")
    {
        REQUIRE_FALSE(cp.stopped());

        REQUIRE(0 == cp.blocked_threads());

        std::thread t([&](){
            const auto result = cp.run_one();
            REQUIRE_FALSE(result);
        });

        // wait for the thread t to block on run_one()
        while (0 == cp.blocked_threads())
        {
            std::this_thread::yield();
        }

        cp.stop();

        t.join();

        REQUIRE(cp.stopped());

        REQUIRE(0 == cp.blocked_threads());

        REQUIRE(0 == cp.ready_handlers());
    }

    SECTION("The run() method blocks the calling thread until stop is called")
    {
        REQUIRE_FALSE(cp.stopped());

        std::thread t([&](){
            const auto count = cp.run();
            REQUIRE(0 == count);
        });

        cp.stop();

        t.join();

        REQUIRE(cp.stopped());

        REQUIRE(0 == cp.blocked_threads());

        REQUIRE(0 == cp.ready_handlers());
    }

    SECTION("Method run_one() processes ready handlers")
    {
        REQUIRE_FALSE(cp.stopped());

        event e;

        std::thread t([&](){
            const auto result = cp.run_one();
            REQUIRE(result);
        });

        // wait for the thread t to block on run()
        while (0 == cp.blocked_threads())
        {
            std::this_thread::yield();
        }

        cp.post([&](const generic_error&) {
            e.notify_all();
        });

        e.wait();

        cp.stop();

        t.join();

        REQUIRE(cp.stopped());

        REQUIRE(0 == cp.blocked_threads());

        REQUIRE(0 == cp.ready_handlers());
    }

    SECTION("Method run() processes ready handlers")
    {
        REQUIRE_FALSE(cp.stopped());

        event e;

        std::thread t([&](){
            const auto count = cp.run();
            REQUIRE(1 == count);
        });

        // wait for the thread t to block on run()
        while (0 == cp.blocked_threads())
        {
            std::this_thread::yield();
        }

        cp.post([&](const generic_error&) {
            e.notify_all();
        });

        e.wait();

        cp.stop();

        t.join();

        REQUIRE(cp.stopped());

        REQUIRE(0 == cp.blocked_threads());

        REQUIRE(0 == cp.ready_handlers());
    }

    SECTION("Blocking methods do not block if there is ready handlers but the port is stopped")
    {
        cp.post([&](const generic_error& e){
            // completion handler must be called with no error
            REQUIRE_FALSE(e);
        });

        cp.stop();

        REQUIRE(cp.stopped());

        REQUIRE(1 == cp.ready_handlers());

        SECTION("wait() processes ready handlers and returns immediately")
        {
            const auto count = cp.wait();
            REQUIRE(1 == count);
        }

        SECTION("wait_one() processes ready handlers and returns immediately")
        {
            const auto result = cp.wait_one();
            REQUIRE(result);
        }

        SECTION("run() processes ready handlers and returns immediately")
        {
            const auto count = cp.run();
            REQUIRE(1 == count);
        }

        SECTION("run_one() processes ready handlers and returns immediately")
        {
            const auto result = cp.run_one();
            REQUIRE(result);
        }

        SECTION("pull() processes ready handlers and returns immediately")
        {
            const auto count = cp.pull();
            REQUIRE(1 == count);
        }

        SECTION("pull_one() processes ready handlers and returns immediately")
        {
            const auto result = cp.pull_one();
            REQUIRE(result);
        }
    }

    SECTION("Blocking methods will block after the port is reset")
    {
        REQUIRE(0 == cp.ready_handlers());

        cp.stop();

        REQUIRE(cp.stopped());

        cp.reset();

        REQUIRE_FALSE(cp.stopped());

        SECTION("wait() processes ready handlers and returns immediately")
        {
            cp.post([&](const generic_error& e){
                // completion handler must be called with no error
                REQUIRE_FALSE(e);
            });

            const auto count = cp.wait();
            REQUIRE(1 == count);

            REQUIRE(0 == cp.ready_handlers());
        }

        SECTION("wait_one() processes ready handlers and returns immediately")
        {
            cp.post([&](const generic_error& e){
                // completion handler must be called with no error
                REQUIRE_FALSE(e);
            });

            REQUIRE(1 == cp.ready_handlers());

            const auto result = cp.wait_one();
            REQUIRE(result);

            REQUIRE(0 == cp.ready_handlers());
        }

        SECTION("run() processes ready handlers and returns immediately")
        {
            std::thread t([&](){
                const auto count = cp.run();
                REQUIRE(1 == count);
            });

            while (0 == cp.blocked_threads())
            {
                std::this_thread::yield();
            }

            event e;

            cp.post([&](const generic_error&){
                e.notify_one();
            });

            e.wait();

            cp.stop();

            t.join();

            REQUIRE(0 == cp.ready_handlers());
        }

        SECTION("run_one() will block ready handlers and returns immediately")
        {
            REQUIRE(0 == cp.ready_handlers());

            std::thread t([&](){
                const auto result = cp.run_one();
                REQUIRE(result);
            });

            while (0 == cp.blocked_threads())
            {
                std::this_thread::yield();
            }

            cp.post([&](const generic_error&){});

            cp.stop();

            t.join();

            REQUIRE(0 == cp.ready_handlers());
        }

        SECTION("pull() processes ready handlers and returns immediately")
        {
            cp.post([&](const generic_error& e){
                // completion handler must be called with no error
                REQUIRE_FALSE(e);
            });

            REQUIRE(1 == cp.ready_handlers());

            const auto count = cp.pull();
            REQUIRE(1 == count);

            REQUIRE(0 == cp.ready_handlers());
        }

        SECTION("pull_one() processes ready handlers and returns immediately")
        {
            cp.post([&](const generic_error& e){
                // completion handler must be called with no error
                REQUIRE_FALSE(e);
            });

            REQUIRE(1 == cp.ready_handlers());

            const auto result = cp.pull_one();
            REQUIRE(result);

            REQUIRE(0 == cp.ready_handlers());
        }
    }
}
