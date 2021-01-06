file(GLOB SOURCES_TEST ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
foreach(local_file ${SOURCES_TEST} ${SOURCES_HASP_TEST})
  get_filename_component(local_filename ${local_file} NAME_WE)

  add_executable(${local_filename} ${local_file})
  target_include_directories(${local_filename} PRIVATE ${Boost_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS} ${googletest_INCLUDE_DIRS} ${googlemock_INCLUDE_DIRS})
  target_link_libraries(${local_filename} glob-cpp
    ${CMAKE_THREAD_LIBS_INIT}
    ${googletest_MAIN_STATIC_LIBRARIES}
    ${googletest_STATIC_LIBRARIES})
  if (NOT WIN32)
    target_link_libraries(${local_filename} pthread)
  endif()
  add_test(UnitTests ${local_filename})
endforeach()
