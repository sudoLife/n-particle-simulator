# GLOBAL CMAKELISTS
cmake_minimum_required(VERSION 3.17)

project(particles CXX)

add_subdirectory(serial)
add_subdirectory(pthreads)
add_subdirectory(openmp)
add_subdirectory(openmpi)
