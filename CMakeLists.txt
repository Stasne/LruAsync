cmake_minimum_required(VERSION 3.20)
project(cache_test LANGUAGES CXX)

message(STATUS "CMake ${CMAKE_VERSION}")
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE src)
# target_link_libraries(${PROJECT_NAME} PRIVATE chrono)