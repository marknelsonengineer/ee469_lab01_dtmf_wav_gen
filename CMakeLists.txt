###############################################################################
###         University of Hawaii, College of Engineering
### @brief  ee469_lab01_dtmf_wav_gen - EE 469 - Fall 2022
###
### CMake manages the building of source code.
###
### @file CMakeLists.txt
### @version 1.0
###
### @see https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html
###
### @author Mark Nelson <marknels@hawaii.edu>
### @date   04_Oct_2022
###############################################################################

cmake_minimum_required(VERSION 3.20)
project( ee469_lab01_dtmf_wav_gen C)

set(CMAKE_C_STANDARD 17)

add_executable(ee469_lab01_dtmf_wav_gen ee469_lab01_dtmf_wav_gen.c)
target_link_libraries(ee469_lab01_dtmf_wav_gen m)

add_executable(goertzel goertzel.c)
target_link_libraries(goertzel m)
