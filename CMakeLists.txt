cmake_minimum_required(VERSION 3.15)
project(parking_lot_system CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4 /O2 /permissive-)
else()
    add_compile_options(-O2 -Wall -Wextra -pedantic)
endif()

set(CORE_SOURCES
    src/parser.cpp
    src/grid.cpp
    src/distance.cpp
    src/slots.cpp
    src/state.cpp
    src/commands.cpp
    src/formatter.cpp
)

add_library(parking_core STATIC ${CORE_SOURCES})
target_include_directories(parking_core PUBLIC src)

add_executable(parking src/main.cpp)
target_link_libraries(parking PRIVATE parking_core)

enable_testing()

file(GLOB TEST_SOURCES tests/test_*.cpp)
foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_link_libraries(${TEST_NAME} PRIVATE parking_core)
    target_include_directories(${TEST_NAME} PRIVATE src tests)
    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
endforeach()
