cmake_minimum_required(VERSION 2.8)
project(glob)

find_package(Boost REQUIRED COMPONENTS filesystem)
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB SRC ${CMAKE_SOURCE_DIR}/src/*.cc)
add_executable(glob ${SRC})
target_link_libraries(glob Boost::filesystem)

install(TARGETS glob RUNTIME DESTINATION bin)

