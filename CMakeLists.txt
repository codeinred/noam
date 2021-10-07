cmake_minimum_required(VERSION 3.14)

project(
    "noam"
    VERSION 0.8.0
    DESCRIPTION "A Parser Combinator Library Build With C++20"
    HOMEPAGE_URL "https://github.com/codeinred/noam/"
    LANGUAGES CXX
)

add_library(noam INTERFACE)
add_library(noam::noam ALIAS noam)

# For ${CMAKE_INSTALL_<DIR>} variables that are standarized
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_compile_features(
    noam
    INTERFACE
    cxx_std_20
)

target_include_directories(
    noam
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Installation
# See: https://dominikberner.ch/cmake-interface-lib/

install(
    TARGETS noam
    EXPORT noam_Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "noamConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/noamConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/noamConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/noam/cmake
)

install(
    EXPORT noam_Targets
    FILE noamTargets.cmake
    NAMESPACE noam::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/noam/cmake
)

install(FILES "${PROJECT_BINARY_DIR}/noamConfig.cmake"
              "${PROJECT_BINARY_DIR}/noamConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/noam/cmake
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/noam
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


include(helper)

find_or_fetch(
    tuplet
    https://github.com/codeinred/tuplet.git
    main)
target_link_libraries(
    noam
    INTERFACE
    tuplet::tuplet)
if(PROJECT_IS_TOP_LEVEL)
    include(CTest)

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    # Add all files in directory test as tests, and link against noam and
    # threads

    #if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    #    target_compile_options(
    #        test_kilo_tuple
    #        PRIVATE
    #        -fbracket-depth=1000)
    #endif()

    set(BENCHMARK_ENABLE_TESTING OFF)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
    set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF)

    # Find the benchmark library, or download it in build/_deps
    find_or_fetch(
        benchmark
        https://github.com/google/benchmark.git
        main)
    find_or_fetch(
        fmt
        https://github.com/fmtlib/fmt.git
        master)
    # always_fetch(
    #     Catch2
    #     https://github.com/catchorg/catch2.git
    #     devel)
    FetchContent_MakeAvailable(${remote_dependencies})

    add_source_dir(
        src
        noam
        fmt)
    add_source_dir(
        bench
        noam
        benchmark)
    # add_executable(
    #     test_noam
    #     test2/test_noam.cpp)
    # target_link_libraries(
    #     test_noam
    #     noam::noam
    #     fmt::fmt
    #     Catch2::Catch2WithMain)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(CTest)
    add_test_dir(test noam fmt)
    # include(Catch)
    # catch_discover_tests(test_noam)
else()
    # If we're not at toplevel we still need to make remote dependencies available
    FetchContent_MakeAvailable(${remote_dependencies})
endif()


