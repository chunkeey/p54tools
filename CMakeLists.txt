cmake_minimum_required(VERSION 2.8)

project(p54tools)

#if you don't want the full compiler output, remove the following line
#set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(include lib/)

add_subdirectory(lib)

add_subdirectory(src)
