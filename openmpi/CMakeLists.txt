cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

project(openmpi CXX)


set(SRCS 
    src/particles.cpp 
    src/partial_grid.cpp
)


find_package(MPI REQUIRED)
find_package(fmt REQUIRED)
find_package(cxxopts REQUIRED)

add_executable(openmpi openmpi.cpp ${SRCS})

target_include_directories(openmpi PRIVATE include)
target_link_libraries(openmpi fmt::fmt cxxopts::cxxopts MPI::MPI_CXX)
