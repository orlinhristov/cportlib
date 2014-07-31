//
// worker_context_example.cpp
//
// This example demonstrates how to provide resource initialization in worker threads.
//

#include <cport/completion_port.hpp>
#include <cport/task_scheduler.hpp>
#include <iostream>
#include <mutex>
#include <thread>

using namespace cport;

class worker_context
{
public:
    void operator()(task_scheduler::worker_func_prototype fn)
    {
        at_start();
        fn();
        at_exit();
    }
private:
    void at_start()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        std::cout << "Initializing worker thread (" << std::this_thread::get_id() << ")" << std::endl;
    }

    void at_exit()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        std::cout << "Destroying worker thread (" << std::this_thread::get_id() << ")" << std::endl;
    }

    std::mutex mtx_;
};

int main()
{
    completion_port p;
    
    worker_context ctxt;

    // Initialize task_scheduler passing user-defined worker thread context
    task_scheduler ts(p, std::ref(ctxt));

    // schedule a task for asynhronous execution
    ts.async(
        // task handler
        [](generic_error&) { std::cout << "Handler called in thread (" << std::this_thread::get_id() << ")" << std::endl; },
        // completion handler
        [](const generic_error&) { std::cout << "Operation complete." << std::endl; }
    );

    // wait for the completion handler to be called
    p.wait();

    return 0;
}
