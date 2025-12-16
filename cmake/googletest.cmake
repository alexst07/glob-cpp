# Copyright 2017 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
include (ExternalProject)

get_filename_component(GOOGLETEST_BINARY_DIR "${CMAKE_BINARY_DIR}/googletest" ABSOLUTE)
set(googletest_INCLUDE_DIRS ${GOOGLETEST_BINARY_DIR}/src/googletest/googletest/include)
set(googlemock_INCLUDE_DIRS ${GOOGLETEST_BINARY_DIR}/src/googletest/googlemock/include)
set(googletest_VERSION 1.8.0)
set(googletest_URL https://github.com/google/googletest/archive/release-${googletest_VERSION}.zip)
set(googletest_BUILD ${GOOGLETEST_BINARY_DIR}/)

if(WIN32)
  if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
    set(googletest_MAIN_STATIC_LIBRARIES
        ${GOOGLETEST_BINARY_DIR}/src/googletest/googletest/$(Configuration)/gtest_main.lib)
    set(googletest_STATIC_LIBRARIES
        ${GOOGLETEST_BINARY_DIR}/src/googletest/googletest/$(Configuration)/gtest.lib)
  else()
    set(googletest_MAIN_STATIC_LIBRARIES
        ${GOOGLETEST_BINARY_DIR}/src/googletest/googletest/gtest_main.lib)
    set(googletest_STATIC_LIBRARIES
        ${GOOGLETEST_BINARY_DIR}/src/googletest/googletest/gtest.lib)
  endif()
else()
  if(APPLE)
    set(LIB_EXT ".dylib")
  else()
    set(LIB_EXT ".so")
  endif()
  get_filename_component(GTEST_MAIN_LIB "${GOOGLETEST_BINARY_DIR}/src/googletest/googlemock/gtest/libgtest_main${LIB_EXT}" ABSOLUTE)
  get_filename_component(GTEST_LIB "${GOOGLETEST_BINARY_DIR}/src/googletest/googlemock/gtest/libgtest${LIB_EXT}" ABSOLUTE)
  get_filename_component(GMOCK_LIB "${GOOGLETEST_BINARY_DIR}/src/googletest/googlemock/libgmock${LIB_EXT}" ABSOLUTE)
  get_filename_component(GMOCK_MAIN_LIB "${GOOGLETEST_BINARY_DIR}/src/googletest/googlemock/libgmock_main${LIB_EXT}" ABSOLUTE)
  set(googletest_MAIN_STATIC_LIBRARIES ${GTEST_MAIN_LIB})
  set(googletest_STATIC_LIBRARIES ${GTEST_LIB} ${GMOCK_LIB} ${GMOCK_MAIN_LIB})
  set(googletest_LIB_EXT ${LIB_EXT})
endif()

ExternalProject_Add(googletest
    PREFIX googletest
    URL ${googletest_URL}
    DOWNLOAD_DIR "${DOWNLOAD_LOCATION}"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    BUILD_IN_SOURCE 1
    # BUILD_BYPRODUCTS ${googletest_STATIC_LIBRARIES} ${googletest_MAIN_STATIC_LIBRARIES}
    INSTALL_COMMAND ""
    CMAKE_ARGS
       -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
       -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
       -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
       -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
       -DBUILD_SHARED_LIBS=ON
    CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBUILD_GMOCK:BOOL=ON
        -DBUILD_GTEST:BOOL=ON
        -Dgtest_force_shared_crt:BOOL=ON
)
