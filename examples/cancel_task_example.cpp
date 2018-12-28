//
// cancel_task_example.cpp
//
// This example demonstrates how to cancel specific task before execution of its handler.
//

#include <cport/completion_port.hpp>
#include <cport/task_scheduler.hpp>
#include <iostream>
#include <chrono>

int main()
{
    using namespace cport;

    completion_port p;

    // initialize the task_scheduler object with one worker thread to guarantee
    //  that the scheduled tasks will be executed in sequential order. 
    task_scheduler ts(p, 1);

    // schedule a task that will delay processing of the next one for a while.
    ts.async([](generic_error&) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });

    // schedule a task for asynchronous execution
    task_t t = ts.async(
        // task handler
        [](generic_error&) { std::cout << "Hello, World!" << std::endl; }
        ,
        // completion handler
        [](const generic_error& e) {
            if (e.code() == operation_aborted) {
                std::cout << "Task execution aborted." << std::endl;
            }
            else {
                std::cout << "Task execution complete." << std::endl;
            }
        }
    );

    // cancel the task
    ts.cancel(t);

    // wait for the completion handler to be called
    p.wait();

    return 0;
}
