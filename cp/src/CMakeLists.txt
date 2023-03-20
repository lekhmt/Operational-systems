cmake_minimum_required(VERSION 3.21)
project(cp)

set(CMAKE_CXX_STANDARD 17)

find_package(benchmark REQUIRED)
add_executable(cp main.cpp)
target_link_libraries(cp benchmark::benchmark)

