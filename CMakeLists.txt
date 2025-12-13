cmake_minimum_required(VERSION 2.8.12)
project(glob-cpp VERSION 1.0.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

option(BUILD_UNIT_TESTS OFF)
option(BUILD_EXAMPLES OFF)

# Prefer Homebrew Boost on macOS to avoid architecture mismatches
if(APPLE)
  set(BOOST_ROOT "/opt/homebrew/opt/boost" CACHE PATH "Boost root directory")
endif()

find_package(Boost REQUIRED COMPONENTS filesystem)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${Boost_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} INTERFACE ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

if (BUILD_UNIT_TESTS)
  enable_testing()
  include(googletest)
  add_subdirectory(tests)
endif ()

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif ()

