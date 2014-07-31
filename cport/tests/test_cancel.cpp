#include <cportlib/task_scheduler.hpp>
#include <cportlib/completion_port.hpp>

struct test_cancel 
{
    test_cancel() : sum(0)
    {
        completion_port port;
        task_scheduler scheduler(port, std::bind(&test_cancel::init, this), [] () { std::cout << "sayonara" << std::endl; } );
        for (std::size_t i = 1; i < 10; ++i) {
            const task_t id = scheduler.async(
                std::bind(&test_cancel::f, this, i, placeholders::error),
                std::bind(&test_cancel::c, this, i, placeholders::error));

            assert(task_t(i) == id);
        }

        for (std::size_t i = 2; i < 10; i+=2) {
            if (i%2 == 0) {
                scheduler.cancel(task_t(i));
            }
        }

        cv_.notify_all();
        port.run();
        assert(sum == 0);
    }

    void init()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock);
    }

private:
    void f(std::size_t i, std::system_error &)
    {
        sum += i;
    }

    void c(std::size_t i, const std::system_error &e)
    {
        if (i%2 == 0)
            assert(e.code().value() == operation_aborted);
        else {
            assert(e.code().value() != operation_aborted);
            sum -= i;
        }
    }

    std::atomic_uint32_t sum;
    std::mutex mtx_;
    std::condition_variable cv_;
};