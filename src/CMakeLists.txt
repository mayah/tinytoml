project(toml)

cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

enable_testing()

function(add_cxx_flags flags)
   set(CMAKE_CXX_FLAGS "${flags} ${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
endfunction()

add_cxx_flags("-Wall -Wextra -std=c++11")
add_cxx_flags("-DSRC_DIR=\\\"${CMAKE_SOURCE_DIR}\\\"")
add_cxx_flags("-DTESTCASE_DIR=\\\"${CMAKE_SOURCE_DIR}/../testcase\\\"")

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
add_toml_test(lexer)
add_toml_test(parser)
add_toml_test(parser_complex)
add_toml_test(parser_failure)
