cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

project(serial CXX)


set(SRCS 
    src/particles.cpp 
    src/grid.cpp
)


find_package(fmt REQUIRED)
find_package(cxxopts REQUIRED)

add_executable(serial serial.cpp ${SRCS})

target_include_directories(serial PRIVATE include)
target_link_libraries(serial fmt::fmt cxxopts::cxxopts)
