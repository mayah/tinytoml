project(toml)

cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Allow IDE's to group targets into folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_testing()

if(${CMAKE_VERSION} VERSION_LESS 2.8.12)
    message("Your cmake version is ${CMAKE_VERSION}. Please update it to 2.8.12 or newer.")
    function(add_compile_options option)
        add_definitions(${option} ${ARGN})
    endfunction()
endif()

if(MSVC)
    # Use mutli-thread runtime library to match with gtest's one.
    add_compile_options("/MT$<$<CONFIG:Debug>:d>")
else()
    add_compile_options("-Wall" "-Wextra" "-std=c++11")
endif()

add_definitions(-DSRC_DIR="${CMAKE_SOURCE_DIR}")
add_definitions(-DTESTCASE_DIR="${CMAKE_SOURCE_DIR}/../testcase")


add_subdirectory(third_party/gtest-1.7.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../include)

mark_as_advanced(
gtest_build_samples
gtest_build_tests
gtest_disable_pthreads
gtest_force_shared_crt
)

set_target_properties(gtest gtest_main 
    PROPERTIES FOLDER "Third_party")

if(MSVC)
    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_compile_definitions(gtest PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
        target_compile_definitions(gtest_main PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    endif()
endif()

add_executable(parse_stdin parse_stdin.cc)
add_executable(parse_file parse_file.cc)
add_executable(parse_file2 parse_file_2.cc)

if(MSVC)
	target_compile_definitions(parse_stdin PUBLIC _CRT_SECURE_NO_WARNINGS)
	target_compile_definitions(parse_file PUBLIC _CRT_SECURE_NO_WARNINGS)
	target_compile_definitions(parse_file2 PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

function(add_toml_test target)
    add_executable(${target}_test ${target}_test.cc ../include/toml/toml.h)
    target_link_libraries(${target}_test gtest gtest_main)
    add_test(${target}_test ${target}_test)
	set_target_properties(${target}_test
            PROPERTIES
            FOLDER "tests")
	if(MSVC)
		target_compile_definitions(${target}_test PUBLIC _CRT_SECURE_NO_WARNINGS)
	endif()
endfunction()

function(add_toml_link_test target)
    add_executable(${target}_test ${target}_test.cc empty.cc ../include/toml/toml.h)
    target_link_libraries(${target}_test gtest gtest_main)
    add_test(${target}_test ${target}_test)
	set_target_properties(${target}_test
            PROPERTIES
            FOLDER "tests")
	if(MSVC)
		target_compile_definitions(${target}_test PUBLIC _CRT_SECURE_NO_WARNINGS)
	endif()
endfunction()

add_toml_test(value)
add_toml_test(lexer)
add_toml_test(parser)
add_toml_test(parser_complex)
add_toml_test(parser_failure)
add_toml_test(writer)
add_toml_test(builder)

add_toml_link_test(link)
