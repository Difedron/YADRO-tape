cmake_minimum_required(VERSION 3.27)
project(YADRO_tape)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1
)

FetchContent_MakeAvailable(Catch2)

add_executable(YADRO_tape
        src/main.cpp
        include/ITape.h
        include/TapeConfig.h
        src/TapeConfig.cpp
        include/FileTape.h
        src/FileTape.cpp
        include/TapeSorter.h
        src/TapeSorter.cpp
)

target_include_directories(YADRO_tape PRIVATE include)

add_executable(YADRO_tape_tests
        tests/tape_tests.cpp
        include/ITape.h
        include/TapeConfig.h
        src/TapeConfig.cpp
        include/FileTape.h
        src/FileTape.cpp
        include/TapeSorter.h
        src/TapeSorter.cpp
)

target_include_directories(YADRO_tape_tests PRIVATE include)

target_link_libraries(YADRO_tape_tests PRIVATE Catch2::Catch2WithMain)