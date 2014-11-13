project(toml)

cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

enable_testing()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11")

add_subdirectory(third_party/gtest-1.7.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../include)

add_executable(parse_stdin parse_stdin.cc)

function(add_toml_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    add_test(${target}_test ${target}_test)
endfunction()

add_toml_test(value)
add_toml_test(parser)
