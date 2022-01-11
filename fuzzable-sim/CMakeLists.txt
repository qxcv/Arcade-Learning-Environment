cmake_minimum_required(VERSION 3.14)

project(example-cpp-lib)

find_package(ale REQUIRED)

add_executable(fuzzableSim fuzzableSim.cpp)
target_link_libraries(fuzzableSim ale::ale-lib)
