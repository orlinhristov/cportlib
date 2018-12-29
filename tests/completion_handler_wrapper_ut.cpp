#include <catch.hpp>
#include <cport/completion_handler_wrapper.hpp>

using namespace cport;

TEST_CASE("Wrapped handler will enforce the port to block on wait() method", "[completion_handler_wrapper]")
{
    completion_port cp;

    const generic_error ge(1, "Oops");

    bool handlerCalled = false;

    REQUIRE(0 == cp.ready_handlers());

    auto count = cp.wait();

    REQUIRE(0 == count);

    auto wrapper = wrap_completion_handler([&](const generic_error& e){
        REQUIRE(e.code() == ge.code());
        handlerCalled = true;
    }, cp);

    REQUIRE(0 == cp.ready_handlers());

    SECTION("Wrapper will post the wrapped completion handler when called")
    {
        wrapper(ge);

        REQUIRE(1 == cp.ready_handlers());

        count = cp.wait();

        REQUIRE(1 == count);

        REQUIRE(handlerCalled);

        SECTION("Wrapper will throw std::bad_function_call if called more than once")
        {
            REQUIRE_THROWS_AS(wrapper(ge), std::bad_function_call);
        }
    }

    SECTION("Completion handler is moved when the wrapper is moved")
    {
        auto wrapper2 = std::move(wrapper);

        REQUIRE_THROWS_AS(wrapper(ge), std::bad_function_call);

        wrapper2(ge);

        REQUIRE(1 == cp.ready_handlers());

        count = cp.wait();

        REQUIRE(1 == count);

        REQUIRE(handlerCalled);
    }

    SECTION("Wrapper will throw if called again after moved")
    {
        wrapper(ge);

        REQUIRE(1 == cp.ready_handlers());

        count = cp.wait();

        REQUIRE(1 == count);

        REQUIRE(handlerCalled);

        auto wrapper2 = std::move(wrapper);

        REQUIRE_THROWS_AS(wrapper2(ge), std::bad_function_call);
    }

    SECTION("Completion handler is not called when wrapper is destoyed")
    {
        {
            auto wrapper2 = std::move(wrapper);
        }

        REQUIRE(1 == cp.ready_handlers());

        count = cp.wait();

        REQUIRE(1 == count);
        
        REQUIRE_FALSE(handlerCalled);
    }
}
