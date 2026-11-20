cmake_minimum_required(VERSION 3.24)
project(gpustat VERSION 1.0.0 LANGUAGES CXX)

option(GPUSTAT_BUILD_TESTS "Build unit tests" ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_path(NVML_INCLUDE_DIR nvml.h
    HINTS /usr/include /usr/local/cuda/include
    REQUIRED)
find_library(NVML_LIBRARY NAMES nvidia-ml
    HINTS /usr/lib /usr/lib64 /usr/lib/x86_64-linux-gnu
          /usr/local/cuda/lib64
    REQUIRED)

add_library(gpustat_core
    src/nvml_monitor.cpp
    src/report.cpp)
target_include_directories(gpustat_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${NVML_INCLUDE_DIR})
target_link_libraries(gpustat_core PUBLIC ${NVML_LIBRARY})
target_compile_options(gpustat_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion;-Wshadow;-Werror>)

add_executable(gpustat src/main.cpp)
target_link_libraries(gpustat PRIVATE gpustat_core)
target_compile_options(gpustat PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion;-Wshadow;-Werror>)

if(GPUSTAT_BUILD_TESTS)
    include(CTest)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        add_executable(gpustat_tests tests/test_report.cpp)
        target_link_libraries(gpustat_tests PRIVATE gpustat_core GTest::gtest_main)
        target_compile_options(gpustat_tests PRIVATE
            $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion;-Wshadow;-Werror>)
        include(GoogleTest)
        gtest_discover_tests(gpustat_tests)
    else()
        message(STATUS "GoogleTest not found; unit tests will not be built")
    endif()
endif()

install(TARGETS gpustat RUNTIME DESTINATION bin)
