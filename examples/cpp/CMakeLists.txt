cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)

add_executable(cpp_server servermain.cpp ../../dstc.c exampleserver.cpp)
add_executable(c_client ../../dstc.c client.c)
add_executable(cpp_client ../../dstc.c cpp_client.cpp)

find_library(RMC rmc HINTS ../../reliable_multicast)

target_link_libraries(cpp_server ${RMC})
target_link_libraries(cpp_client ${RMC})
target_link_libraries(c_client ${RMC})

target_include_directories(cpp_server PUBLIC
../../reliable_multicast ../..)
target_include_directories(c_client PUBLIC
../../reliable_multicast ../..)
target_include_directories(cpp_client PUBLIC
../../reliable_multicast ../..)
