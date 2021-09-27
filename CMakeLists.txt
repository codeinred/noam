# Requirements
cmake_minimum_required(VERSION 3.14)

################################
## Section 1: Declare project ##
################################
set(project_name noam)
set(libname "${project_name}")
project(${project_name} CXX)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

##################################
## Section 2: Find Dependencies ##
##################################
include("${CMAKE_CURRENT_SOURCE_DIR}/helper.cmake")

find_or_fetch(fmt "https://github.com/fmtlib/fmt.git" master)
if(PROJECT_IS_TOP_LEVEL)
    find_or_fetch(benchmark "https://github.com/google/benchmark.git" main)
endif()

FetchContent_MakeAvailable(${remote_dependencies})

###################################
## Section 3: Project CMake code ##
###################################

# Add library and register compile options
add_library(${libname} INTERFACE)

target_cpp_20(${libname})

# Add every directory in the modules/ folder as a subdirectory
add_submodules(${libname} modules)

target_include_directories(
    ${libname}
    INTERFACE
    include/)

if(${CMAKE_PROJECT_NAME} STREQUAL ${project_name})
    set(BENCHMARK_ENABLE_TESTING OFF)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
    set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF)

    add_source_dir(src ${libname} fmt)
    add_source_dir(bench ${libname} benchmark)

    include(CTest)
    add_test_dir(test ${libname} fmt)
endif()
