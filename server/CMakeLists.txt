cmake_minimum_required(VERSION 3.17)
project(dcfsm C)

set(CMAKE_C_STANDARD 11)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(dcserver server.c)
target_link_libraries(dcserver dc_fsm rt)
target_compile_options(dcserver PRIVATE -Wpedantic -Wall -Wextra)

