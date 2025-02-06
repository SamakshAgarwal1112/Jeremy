cmake_minimum_required(VERSION 3.10)
project(Jeremy)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

find_package(Boost REQUIRED)

add_executable(server src/server.cpp)
if(Boost_FOUND)
    target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(server PRIVATE ${Boost_LIBRARIES})
endif()

add_executable(client src/client.cpp)
if(Boost_FOUND)
    target_include_directories(client PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(client PRIVATE ${Boost_LIBRARIES})
endif()
