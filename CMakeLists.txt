cmake_minimum_required(VERSION 2.8)
project(glob)

file(GLOB SRC ${CMAKE_SOURCE_DIR}/src/*.cc)
add_executable(glob ${SRC})

install(TARGETS glob RUNTIME DESTINATION bin)

