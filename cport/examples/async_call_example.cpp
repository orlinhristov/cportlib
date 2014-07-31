//
// async_call_example.cpp
//
// This example demonstrates how to schedule a lambda function for asynhronous call.
// Behaves the same as if bound to a thread:
//  std::thread t([](){ std::cout << "Hello, World!" << std::endl; });
//  t.join();
// 
//

#include <cportlib/completion_port.hpp>
#include <cportlib/task_scheduler.hpp>
#include <iostream>

int main()
{
    using namespace mt;

    completion_port p;

    task_scheduler ts(p);

    // schedule a task for asynhronous execution
    ts.async([] (generic_error&) { std::cout << "Hello, World!" << std::endl; });

    // wait for the task to complete
    p.wait();

    return 0;
}