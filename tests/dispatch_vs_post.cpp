#include <iostream>
#include <completion_port.hpp>

int main()
{
    using namespace mt;
    completion_port port;
    port.post([](const std::system_error&){ std::cout << "post" << std::endl;});
    port.dispatch([](const std::system_error&){ std::cout << "dispatch" << std::endl;});
    port.wait();

    return 0;
}