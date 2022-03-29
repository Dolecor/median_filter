cmake_minimum_required(VERSION 3.0)
project(median_filter LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)


add_executable(median_filter
    src/main.c
    src/median_filter.c
    )

target_include_directories(median_filter
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
