//
// async_call_example2.cpp
//
// This example demonstrates how to schedule a task for asynchronous execution, providing callback handler
//

#include <cport/completion_port.hpp>
#include <cport/task_scheduler.hpp>
#include <iostream>

int main()
{
    using namespace cport;

    completion_port p;

    task_scheduler ts(p);

    // schedule a task for asynchronous execution
    ts.async(
        // task handler
        [] (generic_error&) { std::cout << "Hello, World!" << std::endl; },
        // completion handler
        [] (const generic_error&) { std::cout << "Operation complete." << std::endl; }
    );

    // wait for the completion handler to be called
    p.wait();

    return 0;
}
