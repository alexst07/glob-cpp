# Example executables
add_executable(string-match-example string-match-example.cc)
add_executable(glob-cli glob-cli.cc)
add_executable(wglob-cli wglob-cli.cc)
add_executable(file-glob-example file-glob-example.cc)

target_link_libraries(string-match-example PRIVATE glob-cpp)
target_link_libraries(glob-cli PRIVATE glob-cpp)
target_link_libraries(wglob-cli PRIVATE glob-cpp)
target_link_libraries(file-glob-example PRIVATE glob-cpp)

