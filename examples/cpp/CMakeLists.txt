cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)

add_executable(cpp_server servermain.cpp ../dstc/dstc.c exampleserver.cpp)
add_executable(c_client ../dstc/dstc.c client.c)
add_executable(cpp_client ../dstc/dstc.c cpp_client.cpp)

find_library(RMC rmc HINTS ../dstc/reliable_multicast)

# workaround for void* casting not yet fixed
#target_compile_options(cpp_client PRIVATE -fpermissive)

target_link_libraries(cpp_server ${RMC})
target_link_libraries(cpp_client ${RMC})
target_link_libraries(c_client ${RMC})

target_include_directories(cpp_server PUBLIC
../dstc/reliable_multicast ../dstc)
target_include_directories(c_client PUBLIC
../dstc/reliable_multicast ../dstc)
target_include_directories(cpp_client PUBLIC
../dstc/reliable_multicast ../dstc)
