# Example executable
add_executable(string-match-example string-match-example.cc)

target_link_libraries(string-match-example PRIVATE glob-cpp)

