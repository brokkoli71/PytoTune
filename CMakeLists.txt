cmake_minimum_required(VERSION 4.0)
project(PytoTune)

set(CMAKE_CXX_STANDARD 14)

include_directories(data-structures)

add_executable(PytoTune
        data-structures/Scale.cpp
        data-structures/Scale.h
        playground.cpp
        data-structures/Mode.cpp
        data-structures/Mode.h
        midi-reader/MidiFile.cpp
        midi-reader/MidiFile.h)
