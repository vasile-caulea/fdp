cmake_minimum_required(VERSION 3.25)
project(openmpi_yoyo)

set(CMAKE_CXX_STANDARD 17)

find_package(MPI REQUIRED)
add_executable(openmpi_yoyo sources/main.cpp
        sources/Node.cpp
        headers/Node.hpp
        sources/yoyo_alg.cpp
        headers/Edge.hpp
        headers/yoyo_alg.hpp)

target_link_libraries(openmpi_yoyo PUBLIC MPI::MPI_CXX)
