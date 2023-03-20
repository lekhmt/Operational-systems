project(6lab)

add_executable(server server.cpp message.cpp)
add_executable(client client.cpp message.cpp)

target_link_libraries(server pthread zmq)
target_link_libraries(client pthread zmq)
