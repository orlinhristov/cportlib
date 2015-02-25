//
// wait_example.cpp
//
// This example demonstrates how to wait for specific task to be executed.
//

#ifndef CPORT_ENABLE_TASK_STATUS
#define CPORT_ENABLE_TASK_STATUS
#endif

#include <cport/completion_port.hpp>
#include <cport/task_scheduler.hpp>
#include <iostream>

int main()
{
    using namespace cport;

    completion_port p;

    task_scheduler ts(p);

    // schedule a task for asynhronous execution
    task_t t = ts.async([](generic_error&) { std::cout << "Hello, World!" << std::endl; });

    // block until task handler is executed
    t.wait();
    return 0;
}
