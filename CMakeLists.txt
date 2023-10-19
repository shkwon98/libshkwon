cmake_minimum_required( VERSION 3.14 )

project( shkwon 
         LANGUAGES CXX )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library( shkwon INTERFACE )
target_include_directories( shkwon
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include )
add_library( shkwon::shkwon ALIAS shkwon )