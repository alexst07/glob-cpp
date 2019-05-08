include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories( ${GTEST_INCLUDE_DIRS})

file(GLOB SOURCES_TEST ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
foreach(local_file ${SOURCES_TEST} ${SOURCES_HASP_TEST})
  get_filename_component(local_filename ${local_file} NAME_WE)

  add_executable(${local_filename} ${local_file})
  target_link_libraries(${local_filename}
    ${CMAKE_THREAD_LIBS_INIT}
    ${googletest_MAIN_STATIC_LIBRARIES}
    ${googletest_STATIC_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY})
  add_test(UnitTests ${local_filename})
endforeach()