cmake_minimum_required(VERSION 3.24)
project(gpustat VERSION 1.1.0 LANGUAGES CXX)

option(GPUSTAT_BUILD_TESTS "Build unit tests when GoogleTest is available" ON)

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

function(gpustat_enable_warnings target_name)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
            -Wshadow -Werror)
    endif()
endfunction()

gpustat_enable_warnings(gpustat_core)

add_executable(gpustat src/main.cpp)
target_link_libraries(gpustat PRIVATE gpustat_core)
gpustat_enable_warnings(gpustat)

if(GPUSTAT_BUILD_TESTS)
    include(CTest)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        add_executable(gpustat_tests tests/test_report.cpp)
        target_link_libraries(gpustat_tests PRIVATE gpustat_core GTest::gtest_main)
        gpustat_enable_warnings(gpustat_tests)
        include(GoogleTest)
        gtest_discover_tests(gpustat_tests)
    else()
        message(STATUS "GoogleTest not found; gpustat_tests will not be built")
    endif()
endif()

install(TARGETS gpustat RUNTIME DESTINATION bin)
