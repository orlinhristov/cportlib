cmake_minimum_required (VERSION 3.0)
project (UnitTest)
include_directories("./")
include_directories("../")
add_definitions(-DCPORT_HEADER_ONLY_LIB)
add_executable(unit_test completion_port_ut.cpp completion_handler_wrapper_ut.cpp task_scheduler_ut.cpp task_channel_ut.cpp event_ut.cpp main_ut.cpp)
add_executable(perf_test perf_test.cpp)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR
    ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
        set(warnings "-Wall")
endif()
