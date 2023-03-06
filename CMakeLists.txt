cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

project(particles CXX)

# add_executable(${PROJECT_NAME} pparticles.cpp common.cpp)
# add_executable(serial serial.cpp common.cpp)


set(SRCS 
    src/particles.cpp 
    src/grid.cpp
)

# settings this globally becaus we'll need it everywhere
include_directories(include)


find_package(fmt REQUIRED)
find_package(cxxopts REQUIRED)

add_executable(serial serial.cpp ${SRCS})
# TODO: build an interface library
target_link_libraries(serial fmt::fmt cxxopts::cxxopts)


# SECTION FOR OLD EXECUTABLES
add_subdirectory(particles)
