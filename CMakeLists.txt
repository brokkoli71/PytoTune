cmake_minimum_required(VERSION 3.9)
project(PytoTune LANGUAGES CXX)

# Compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

# Add the library (core DSP logic)
add_library(pytotune_core STATIC
        src/io/wav_file.cpp
        src/data-structures/mode.cpp
        src/data-structures/scale.cpp
        src/io/midi_file.cpp
        src/algorithms/fft.cpp
        src/algorithms/yin_pitch_detector.cpp
        src/algorithms/pitch_shifter.cpp
        src/data-structures/windowing.cpp
)

set_target_properties(pytotune_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(pytotune_core PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(pytotune_core PUBLIC OpenMP::OpenMP_CXX)

add_executable(pytotune_cli
        src/main.cpp
)
target_compile_options(pytotune_cli PUBLIC -Ofast)
target_link_libraries(pytotune_cli PUBLIC pytotune_core)


# GoogleTest
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Pybind11
FetchContent_Declare(
        pybind11
        URL https://github.com/pybind/pybind11/archive/v2.11.1.zip
)
FetchContent_MakeAvailable(pybind11)

# Pybind11 module
pybind11_add_module(pytotune src/python_bindings.cpp)
target_link_libraries(pytotune PRIVATE pytotune_core)

# Add the test executable
add_executable(pytotune_tests tests/io/test_wav_file.cpp
        tests/test_utils.h
        tests/io/test_midi_file.cpp
        tests/data-structures/test_scale.cpp
        tests/algorithms/test_pitch_shifter.cpp
        tests/data-structures/test_scale.cpp
        tests/algorithms/pitch_detection.cpp
        tests/algorithms/test_fft.cpp
        tests/reference/pitch_shifter_reference.cpp
        tests/reference/pitch_shifter_reference.h
)
target_link_libraries(pytotune_tests PRIVATE pytotune_core gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(pytotune_tests)
