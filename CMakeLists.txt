cmake_minimum_required(VERSION 3.5.0)
project(cpp-server VERSION 0.1.0 LANGUAGES C CXX)

include_directories(${PROJECT_SOURCE_DIR}/src)

# build server
add_executable(server src/server.cpp src/utils.cpp)

# build client
add_executable(client src/client.cpp src/utils.cpp)
