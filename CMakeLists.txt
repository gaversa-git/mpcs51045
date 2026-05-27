# Build configuration for the CAN framework.

cmake_minimum_required(VERSION 3.20)

project(can_framework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(can_framework INTERFACE)

target_include_directories(can_framework INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/messages
)

add_executable(can_demo src/main.cpp)
target_link_libraries(can_demo PRIVATE can_framework)

add_executable(test_phase1 tests/test_phase1.cpp)
target_link_libraries(test_phase1 PRIVATE can_framework)

add_executable(test_roundtrip tests/test_roundtrip.cpp)
target_link_libraries(test_roundtrip PRIVATE can_framework)