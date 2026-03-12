#include "HelloTriangleApplication.hpp"
#include <stdexcept>
#include <iostream>
#include <cstdlib>

//for PID
#include <unistd.h>
int main(int argc, char const *argv[])
{
    pid_t pid = getpid();
    std::cout << "process ID: " << pid << std::endl;

    std::string processName = argv[0];
    std::cout << "Process Name: " << processName << std::endl;
    HelloTriangleApplication application(processName);
    try
    {
        application.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
    
}
