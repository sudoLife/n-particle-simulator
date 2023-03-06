# GLOBAL CMAKELISTS
cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

project(particles CXX)

add_subdirectory(serial)
add_subdirectory(pthreads)
