cmake_minimum_required(VERSION 3.20)
project(5lab)

set(CMAKE_C_STANDARD 11)

# static
add_executable(static static.c)

add_library(static_euclid_gcf STATIC lib/euclid_gcf.c)
target_link_libraries(static static_euclid_gcf)

add_library(static_naive_gcf STATIC lib/naive_gcf.c)
# target_link_libraries(static static_naive_gcf)

add_library(static_rectangle_sin STATIC lib/rectangle_sin.c)
target_link_libraries(static static_rectangle_sin)

add_library(static_trapezoid_sin STATIC lib/trapezoid_sin.c)
# target_link_libraries(static static_trapezoid_sin)

target_link_libraries(static m)

# dynamic
add_executable(dynamic dynamic.c)

add_library(dynamic_euclid_gcf SHARED lib/euclid_gcf.c)
add_library(dynamic_naive_gcf SHARED lib/naive_gcf.c)

add_library(dynamic_rectangle_sin SHARED lib/rectangle_sin.c)
target_link_libraries(dynamic_rectangle_sin m)
add_library(dynamic_trapezoid_sin SHARED lib/trapezoid_sin.c)
target_link_libraries(dynamic_trapezoid_sin m)

target_link_libraries(dynamic dl)
