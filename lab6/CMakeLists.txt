cmake_minimum_required(VERSION 3.5)
project(lab6 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_ALL "Build all example programs" ON)

# List of example sources (each will become its own executable)
set(EXAMPLES
    check_thread.c
    simple_thread.c
    thread_cond_variables.c
    thread_increment_mutex.c
    thread_cancel.c
    detached_thread.c
    elements.c
    thread_increment.c
    task1_simple_return.c
    task3_detach.c
    thread_increment_ordered.c
    task8_chain.c
    task9_fill.c
    task10_sum.c
)

if(BUILD_ALL)
    foreach(src ${EXAMPLES})
        get_filename_component(name ${src} NAME_WE)
        add_executable(${name} ${src})
        target_compile_options(${name} PRIVATE -Wall -Wextra)
        target_link_libraries(${name} PRIVATE pthread)
    endforeach()
endif()

# Helper target to build everything
add_custom_target(build_all DEPENDS ${EXAMPLES})

message(STATUS "Configured lab6: BUILD_ALL=${BUILD_ALL}")
