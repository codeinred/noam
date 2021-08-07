# Requirements
cmake_minimum_required(VERSION 3.10)

# Get the project name as the name of the current source directory
# Feel free to change this if your project has a different name
get_filename_component(project_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${project_name} CXX)
set(libname "${project_name}")
set(working_dir "${CMAKE_CURRENT_SOURCE_DIR}")

# Use pthreads, if availible
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Enable threads in project
find_package(Threads REQUIRED)

message("Building project ${project_name}")

# Add library and register compile options
add_library(${libname} INTERFACE)
set_property(TARGET ${libname} PROPERTY CXX_STANDARD_REQUIRED 20)
target_compile_features(${libname} INTERFACE cxx_std_20)

# The /EHa flag enables standard C++ stack unwinding
# See: https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=msvc-160
if (MSVC)
    target_compile_options(${libname} INTERFACE "/EHa")
endif()

# Enables GCC support for coroutines (these are standard C++ now but GCC still
# requires a flag for them)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    target_compile_options(${libname} INTERFACE "-fcoroutines")
endif()


# Add every directory in the modules/ folder as a subdirectory
if(EXISTS "${working_dir}/modules/")
    file(GLOB all_modules "modules/*")
    foreach(module_dir ${all_modules})
        get_filename_component(module ${module_dir} NAME)
        message("Adding ${module} @ modules/${module}")
        add_subdirectory("modules/${module}")
        target_link_libraries(${libname} INTERFACE ${module})
    endforeach()
endif()


# target_compile_definitions(${libname} INTERFACE)

target_include_directories(
    ${libname}
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>
)

# Provides install target, so users can install the library on their system
# if they like

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION include
)

# If we're building this project, and not something that has this project as a dependency
# Then we want to build files in the src directory. This will build every top-level
# file in the src directory as a separate program
if((${CMAKE_PROJECT_NAME} STREQUAL ${project_name}) AND (EXISTS "${working_dir}/src"))
    file(GLOB all_executables "src/*.cpp")

    foreach(file_name ${all_executables})
        get_filename_component(executable_name ${file_name} NAME_WLE)
        message("Adding ${executable_name} from src/")
        add_executable("${executable_name}" "${file_name}")
        target_link_libraries("${executable_name}" PRIVATE ${libname})
        target_link_libraries("${executable_name}" PRIVATE Threads::Threads)
    endforeach()
endif()

# Build every file in the test/ directory as a separate program and register
# it as a test
if((EXISTS "${working_dir}/test/") AND (${CMAKE_PROJECT_NAME} STREQUAL ${project_name}) AND BUILD_TESTING)
    include(CTest)

    file(GLOB all_tests "test/*.cpp")

    foreach(file_name ${all_tests})
        get_filename_component(test_name ${file_name} NAME_WLE)
        add_executable("${test_name}" "${file_name}")
        target_link_libraries("${test_name}" PRIVATE ${libname})
        target_link_libraries("${test_name}" PRIVATE Threads::Threads)
        add_test(NAME "${test_name}" COMMAND "${test_name}")
        message("[test: ${test_name}]")
    endforeach()
endif()

# We want to export compile commands b/c it's useful for ccls
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
