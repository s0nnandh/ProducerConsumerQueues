cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_C_LINK_EXECUTABLE /opt/clang+llvm/clang+llvm-17.0.6/bin/lld)

project(helloWorld)

function(print_target_properties target)
    message(STATUS "Compilation properties for target ${target}:")
    
    # Get compile options
    get_target_property(opts ${target} COMPILE_OPTIONS)
    if(opts)
        message(STATUS "  COMPILE_OPTIONS: ${opts}")
    else()
        message(STATUS "  COMPILE_OPTIONS: (none)")
    endif()
    
    # Get compile definitions
    get_target_property(defs ${target} COMPILE_DEFINITIONS)
    if(defs)
        message(STATUS "  COMPILE_DEFINITIONS: ${defs}")
    else()
        message(STATUS "  COMPILE_DEFINITIONS: (none)")
    endif()
    
    # Get include directories
    get_target_property(incs ${target} INCLUDE_DIRECTORIES)
    if(incs)
        message(STATUS "  INCLUDE_DIRECTORIES: ${incs}")
    else()
        message(STATUS "  INCLUDE_DIRECTORIES: (none)")
    endif()
    
    # Get compile flags
    get_target_property(flags ${target} COMPILE_FLAGS)
    if(flags)
        message(STATUS "  COMPILE_FLAGS: ${flags}")
    else()
        message(STATUS "  COMPILE_FLAGS: (none)")
    endif()
endfunction()

cmake_path(GET PROJECT_BINARY_DIR STEM buildDir)
string(TOLOWER ${buildDir} buildDir)
if(buildDir STREQUAL "release")
    message("Building project in Release mode")
    set(buildType Release)
elseif(buildDir STREQUAL "build")
    message("Building project in Debug mode")
    set(buildType Debug)
else()
    set(buildType Debug)
endif()
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${buildType})
endif()

set(SOURCES src/helloWorld.cc)

# add_link_options(-v -fsanitize=thread)

# Define a function to add an executable with the given name and source files
function(add_custom_executable target_name sources)
    # Create the executable target
    add_executable(${target_name} ${sources})

    # Global compilation options
    target_compile_options(${target_name}
    PUBLIC
        -Wall
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Debug-specific options
        target_compile_options(${target_name}
        PUBLIC
            -g
            -ferror-limit=1
            -fsanitize=thread,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(${target_name} PRIVATE -fsanitize=undefined -fsanitize=thread)

        # Add an additional executable for thread sanitizer
        set(tsan_target_name ${target_name}.tsan)
        add_executable(${tsan_target_name} ${sources})
        target_include_directories(${tsan_target_name} PUBLIC ${PROJECT_SOURCE_DIR}/lib)
        target_compile_options(${tsan_target_name} PRIVATE -fsanitize=thread)
        target_link_options(${tsan_target_name} PRIVATE -fsanitize=thread)
        target_link_libraries(${tsan_target_name} PRIVATE pthread)
    else()
        # Release-specific options
        target_compile_options(${target_name}
        PUBLIC
            -march=native
            -flto
        )
        # Add an additional executable for thread sanitizer
        set(tsan_target_name ${target_name}.tsan)
        add_executable(${tsan_target_name} ${sources})
        target_include_directories(${tsan_target_name} PUBLIC ${PROJECT_SOURCE_DIR}/lib)
        target_compile_options(${tsan_target_name} PRIVATE -fsanitize=thread)
        target_link_options(${tsan_target_name} PRIVATE -fsanitize=thread)
        target_link_libraries(${tsan_target_name} PRIVATE pthread benchmark::benchmark benchmark::benchmark_main)
    endif()

    # Include directories and link libraries
    target_include_directories(${target_name} PUBLIC ${PROJECT_SOURCE_DIR}/lib)
endfunction()

add_custom_executable(${PROJECT_NAME} src/helloWorld.cc)

# Unit testing
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

FetchContent_MakeAvailable(googletest)

add_executable(
  tests
  tst/unitTests.cpp
)

target_include_directories(tests PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_compile_options(tests PRIVATE -fsanitize=undefined -fsanitize=thread)
target_link_libraries(tests PRIVATE gtest gtest_main)
target_link_options(tests PRIVATE -fsanitize=undefined -fsanitize=thread)

function(add_benchmark_executable target_name sources)
    add_custom_executable(${target_name} ${sources})
    target_link_libraries(${target_name} PRIVATE benchmark::benchmark benchmark::benchmark_main)
endfunction()

# Google Benchmark
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.4 
)

FetchContent_MakeAvailable(benchmark)

add_benchmark_executable(random_add_bench benchmarks/test_random_add.cc)
add_benchmark_executable(benchmark_queue benchmarks/benchmark_queue.cc)

message(STATUS "Build type: '${CMAKE_BUILD_TYPE}'")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")
print_target_properties(${PROJECT_NAME})