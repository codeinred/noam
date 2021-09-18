# Requirements
cmake_minimum_required(VERSION 3.14)

################################
## Section 1: Declare project ##
################################
set(project_name noam)
set(libname "${project_name}")
project(${project_name} CXX)

include("${CMAKE_CURRENT_SOURCE_DIR}/helper.cmake")
###################################
## Section 2: Project CMake code ##
###################################

# Add library and register compile options
add_library(${libname} INTERFACE)

target_cpp_20(${libname})

# Add every directory in the modules/ folder as a subdirectory
add_submodules(${libname} modules)

target_include_directories(
    ${libname}
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION include
)

if(${CMAKE_PROJECT_NAME} STREQUAL ${project_name})
    set(BENCHMARK_ENABLE_TESTING OFF)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
    set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF)
    find_or_fetch(fmt "https://github.com/fmtlib/fmt.git" master)
    find_or_fetch(benchmark "https://github.com/google/benchmark.git" main)

    add_source_dir(src ${libname} fmt)
    add_source_dir(bench ${libname} benchmark)
    add_test_dir(test ${libname} fmt)
endif()
