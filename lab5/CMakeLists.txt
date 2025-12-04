cmake_minimum_required(VERSION 3.5)
project(lab5 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(USE_MAP_ANON "Build `anon_mmap` with MAP_ANONYMOUS (define USE_MAP_ANON)")

add_compile_options(-Wall -Wextra)

# Include headers from writer-reader
include_directories(${CMAKE_SOURCE_DIR}/writer-reader)

# Put all runtime executables into build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Root utilities
add_executable(mcat mcat.c)
add_executable(mmap mmap.c)
add_executable(anon_mmap anon_mmap.c)

if(USE_MAP_ANON)
    target_compile_definitions(anon_mmap PRIVATE USE_MAP_ANON)
endif()

# binary_sems implemented as a small static library used by writer/reader
add_library(binary_sems STATIC writer-reader/binary_sems.c)
target_include_directories(binary_sems PUBLIC ${CMAKE_SOURCE_DIR}/writer-reader)

add_executable(writer writer-reader/writer.c)
target_link_libraries(writer PRIVATE binary_sems)
target_include_directories(writer PRIVATE ${CMAKE_SOURCE_DIR}/writer-reader)

add_executable(reader writer-reader/reader.c)
target_link_libraries(reader PRIVATE binary_sems)
target_include_directories(reader PRIVATE ${CMAKE_SOURCE_DIR}/writer-reader)

# Simple cp utility
add_executable(cp_simple cp_simple.c)

# Helpful targets/vars
set_target_properties(mcat mmap anon_mmap writer reader PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message(STATUS "lab5 project configured. USE_MAP_ANON=${USE_MAP_ANON}")
