cmake_minimum_required(VERSION 3.20)
project(3lab C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-pthread")

add_executable(3lab main.c)
