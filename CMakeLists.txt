cmake_minimum_required(VERSION 3.14)
project(PytoTune LANGUAGES CXX)

# Compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# GoogleTest
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add the library (core DSP logic)
add_library(pytotune_core
        src/io/wav_file.cpp
        src/data-structures/mode.cpp
        src/data-structures/scale.cpp
        src/io/midi_file.cpp
        src/algorithms/fft.cpp
        src/algorithms/yin_pitch_detector.cpp
        src/algorithms/pitch_shifter.cpp
        src/data-structures/windowing.cpp
)

target_include_directories(pytotune_core PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Add the test executable
add_executable(tests tests/io/test_wav_file.cpp
        tests/test_utils.h
        tests/io/test_midi_file.cpp
        tests/data-structures/test_scale.cpp
        src/algorithms/fft.cpp
        tests/algorithms/test_pitch_shifter.cpp
        tests/data-structures/test_scale.cpp
        tests/algorithms/pitch_detection.cpp
        tests/algorithms/test_fft.cpp
        src/data-structures/windowing.cpp
)
target_link_libraries(tests PRIVATE pytotune_core gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(tests)
