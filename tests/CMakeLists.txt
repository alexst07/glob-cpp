file(GLOB SOURCES_TEST ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

# Build tests in both noexcept mode (default) and exception mode
foreach(local_file ${SOURCES_TEST} ${SOURCES_HASP_TEST})
  get_filename_component(local_filename ${local_file} NAME_WE)

  # Default mode: GLOBCPP_NOEXCEPT_ENABLED=1 (noexcept mode)
  add_executable(${local_filename} ${local_file})
  add_dependencies(${local_filename} googletest)
  target_include_directories(${local_filename} PRIVATE ${Boost_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS} ${googletest_INCLUDE_DIRS} ${googlemock_INCLUDE_DIRS})
  target_compile_definitions(${local_filename} PRIVATE GLOBCPP_NOEXCEPT_ENABLED=1)
  target_link_libraries(${local_filename} glob-cpp
    ${CMAKE_THREAD_LIBS_INIT}
    ${googletest_MAIN_STATIC_LIBRARIES}
    ${googletest_STATIC_LIBRARIES})
  if (NOT WIN32)
    target_link_libraries(${local_filename} pthread)
  endif()
  add_test(${local_filename}_noexcept ${local_filename})

  # Exception mode: GLOBCPP_NOEXCEPT_ENABLED=0 (exception mode)
  add_executable(${local_filename}_throws ${local_file})
  add_dependencies(${local_filename}_throws googletest)
  target_include_directories(${local_filename}_throws PRIVATE ${Boost_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS} ${googletest_INCLUDE_DIRS} ${googlemock_INCLUDE_DIRS})
  target_compile_definitions(${local_filename}_throws PRIVATE GLOBCPP_NOEXCEPT_ENABLED=0)
  target_link_libraries(${local_filename}_throws glob-cpp
    ${CMAKE_THREAD_LIBS_INIT}
    ${googletest_MAIN_STATIC_LIBRARIES}
    ${googletest_STATIC_LIBRARIES})
  if (NOT WIN32)
    target_link_libraries(${local_filename}_throws pthread)
  endif()
  add_test(${local_filename}_throws ${local_filename}_throws)
endforeach()
