//
// wait_example.cpp
//
// This example demonstrates how to wait for specific task to be executed.
//

#define CPORT_ENABLE_TASK_STATUS
#include <cportlib/completion_port.hpp>
#include <cportlib/task_scheduler.hpp>
#include <iostream>

int main()
{
    using namespace mt;

    completion_port p;

    task_scheduler ts(p);

    // schedule a task for asynhronous execution
    task_t t = ts.async([](generic_error&) { std::cout << "Hello, World!" << std::endl; });

    // block until task handler is executed
    t.wait();
    return 0;
}