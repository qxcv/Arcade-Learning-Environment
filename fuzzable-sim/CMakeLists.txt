cmake_minimum_required(VERSION 3.14)

project(fuzzable-sim)

find_package(ale REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(X11 REQUIRED)

add_executable(fuzzableSim fuzzableSim.cpp)
target_link_libraries(fuzzableSim ale::ale-lib ${X11_LIBRARIES})

add_executable(recordVideo recordVideo.cpp)
target_link_libraries(recordVideo ale::ale-lib ${X11_LIBRARIES})
