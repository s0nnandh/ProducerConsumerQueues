cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS OFF)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    SET(CMAKE_C_LINK_EXECUTABLE /opt/clang+llvm/clang+llvm-17.0.6/bin/lld)
endif()

project(helloWorld)

function(print_target_properties target)
    message(STATUS "Computer architecture ${CMAKE_SYSTEM_PROCESSOR}")
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

SET(BOOST_VERSION 1.71)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
	SET(BOOST_VERSION 1.85)
endif()

find_package(Boost ${BOOST_VERSION} REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

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
        -fno-omit-frame-pointer
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Debug-specific options
        target_compile_options(${target_name}
        PUBLIC
            -g
            -fsanitize=thread,undefined
        )
        target_link_options(${target_name} PRIVATE -fsanitize=undefined -fsanitize=thread)

        # Add an additional executable for thread sanitizer
        # set(tsan_target_name ${target_name}.tsan)
        # add_executable(${tsan_target_name} ${sources})
        # target_include_directories(${tsan_target_name} PUBLIC ${PROJECT_SOURCE_DIR}/lib)
        # target_compile_options(${tsan_target_name} PRIVATE -fsanitize=thread)
        # target_link_options(${tsan_target_name} PRIVATE -fsanitize=thread)
        # target_link_libraries(${tsan_target_name} PRIVATE pthread benchmark::benchmark benchmark::benchmark_main)
    else()
        # Release-specific options
        target_compile_options(${target_name}
        PUBLIC
            -march=native
            -mtune=native
            -flto
        )
    endif()

    # Include directories and link libraries
    # target_include_directories(${target_name} PUBLIC ${PROJECT_SOURCE_DIR}/lib)
endfunction()

# add_custom_executable(${PROJECT_NAME} src/helloWorld.cc)

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

target_include_directories(tests PRIVATE ${PROJECT_SOURCE_DIR}/lib)
target_compile_options(tests PRIVATE -fsanitize=undefined -fsanitize=thread)
target_link_libraries(tests PRIVATE gtest gtest_main)
target_link_options(tests PRIVATE -fsanitize=undefined -fsanitize=thread)

function(add_benchmark_executable target_name sources)
    add_custom_executable(${target_name} ${sources})
    target_link_libraries(${target_name} PRIVATE -v benchmark::benchmark)
    target_include_directories(${target_name} PUBLIC ${PROJECT_SOURCE_DIR}/lib)
endfunction()

# Google Benchmark
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
message("Fetching google benchmark")
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.4 
)

FetchContent_MakeAvailable(benchmark)
else()
    message("Using local benchmark")
	find_package(benchmark REQUIRED)
endif()

# add_benchmark_executable(random_add_bench benchmarks/test_random_add.cc)
add_benchmark_executable(benchmark_queue benchmarks/benchmark_queue.cc)
add_benchmark_executable(basic_spsc_queue src/BasicSPSC.cc)
add_benchmark_executable(basic_spsc_without_modulo_queue src/BasicSPSCWithoutModulo.cc)
add_benchmark_executable(spsc_ra_pairs src/SPSCWithRAPairs.cc)
add_benchmark_executable(spsc_without_fs src/SPSCWithoutFS.cc)
add_benchmark_executable(spsc_local_cache src/SPSCLocal.cc)
add_benchmark_executable(rigtorp_spsc src/rigtorp.cc)


add_executable(bench benchmarks/benchmark_queue.cc)
target_link_libraries(bench PRIVATE benchmark::benchmark)
target_include_directories(bench PUBLIC ${PROJECT_SOURCE_DIR}/lib)

message(STATUS "Build type: '${CMAKE_BUILD_TYPE}'")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")
# print_target_properties(${PROJECT_NAME})
