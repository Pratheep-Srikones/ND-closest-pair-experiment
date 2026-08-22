cmake_minimum_required(VERSION 3.10)

project(ND-closest-pair-experiment)

set(CMAKE_CXX_STANDARD 17)                                                                                                                       
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_executable(run main.cpp)
target_link_libraries(run tbb)
