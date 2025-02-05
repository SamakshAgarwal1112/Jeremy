cmake_minimum_required(VERSION 3.10)
project(Jeremy)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")
add_executable(Jeremy ${SOURCES})

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(Jeremy ${Boost_LIBRARIES})
endif()
