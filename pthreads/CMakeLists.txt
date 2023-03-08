cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 20)

project(threads CXX)


set(SRCS 
    src/particles.cpp 
    src/grid.cpp
)


find_package(fmt REQUIRED)
find_package(cxxopts REQUIRED)

add_executable(threads threads.cpp ${SRCS})

target_include_directories(threads PRIVATE include)
target_link_libraries(threads fmt::fmt cxxopts::cxxopts)
