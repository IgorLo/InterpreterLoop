cmake_minimum_required(VERSION 3.1)
project(InterpreterLoop C)

set(CMAKE_C_STANDARD 99)

add_executable(InterpreterLoop main.c main.h loader.c)
set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")