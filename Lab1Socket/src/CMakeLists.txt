cmake_minimum_required(VERSION 3.0)


project(MyProject)

add_executable(client client.cpp)
add_executable(server server.cpp)

target_link_libraries(client wsock32)
target_link_libraries(server wsock32)

