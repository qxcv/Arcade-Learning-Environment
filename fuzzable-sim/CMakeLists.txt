cmake_minimum_required(VERSION 3.14)

project(fuzzable-sim)

set(CMAKE_CXX_STANDARD 17)

find_package(ale REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

add_executable(fuzzableSim fuzzableSim.cpp)
target_link_libraries(fuzzableSim ale::ale-lib)

add_executable(recordVideo recordVideo.cpp)
target_link_libraries(recordVideo ale::ale-lib)
