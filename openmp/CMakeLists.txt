cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

project(openmp CXX)


set(SRCS 
    src/particles.cpp 
    src/grid.cpp
)


find_package(fmt REQUIRED)
find_package(cxxopts REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(openmp openmp.cpp ${SRCS})

target_include_directories(openmp PRIVATE include)
target_link_libraries(openmp fmt::fmt cxxopts::cxxopts OpenMP::OpenMP_CXX)
