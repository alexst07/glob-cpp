cmake_minimum_required(VERSION 2.8)
project(glob)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

option(BUILD_UNIT_TESTS OFF)

find_package(Boost REQUIRED COMPONENTS filesystem)
include_directories(${Boost_INCLUDE_DIRS})

if (BUILD_UNIT_TESTS)
  enable_testing()
  include(googletest)
  include_directories(SYSTEM ${googletest_INCLUDE_DIRS}
                      ${googlemock_INCLUDE_DIRS})
  add_subdirectory(tests)
endif ()
