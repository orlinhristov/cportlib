//
// cancel_task_example.cpp
//
// This example demonstrates how to cancel specific task before execution of its handler.
//

#include <cportlib/completion_port.hpp>
#include <cportlib/task_scheduler.hpp>
#include <iostream>
#include <mutex>
#include <condition_variable>

int main()
{
    using namespace mt;

    std::mutex m;

    std::condition_variable cv;

    completion_port p;

    // initialize task_scheduler passing user-defined worker thread context
    task_scheduler ts(p, [&](task_scheduler::worker_func_prototype fn) {
        // block the worker thread startup
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock);
        lock.unlock();
        // call the worker
        fn();
    });

    // schedule a task for asynhronous execution
    task_t t = ts.async(
        // task handler
        [](generic_error&) { std::cout << "Hello, World!" << std::endl; },
        // completion handler
        [](const generic_error& e) { std::cout << e.what() << std::endl; }
    );

    // cancel the task
    ts.cancel(t);
    
    // unblock worker threads
    cv.notify_all();

    // wait for the completion handler to be called
    p.wait();

    return 0;
}