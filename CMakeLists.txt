cmake_minimum_required(VERSION 3.20.0 FATAL_ERROR)
project(red_diamond_cmake LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(HDF5 COMPONENTS C CXX HL REQUIRED)
set(CMAKE_THREAD_PREFER_PTHREAD On)
set(THREADS_PREFER_PTHREAD_FLAG On)
find_package(Threads REQUIRED)

add_subdirectory(src)



# The main red_diamond executable.
add_executable(red_diamond ${PROJECT_SOURCE_DIR}/src/red_diamond_main.c)

target_link_libraries(red_diamond PRIVATE libreddiamond libz4 
                                hdf5::hdf5 hdf5::hdf5_cpp hdf5::hdf5_hl
                )        

#Compilation settings
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(red_diamond PUBLIC -Wall -Wextra -pedantic)
endif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")

if (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    target_compile_options(red_diamond PUBLIC -Wall -Wextra -pedantic)
    target_compile_options(red_diamond PUBLIC -xHost)
endif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")