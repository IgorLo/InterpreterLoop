cmake_minimum_required(VERSION 3.9.6)
project(InterpreterLoop C)

set(CMAKE_C_STANDARD 99)

add_executable(InterpreterLoop main.c main.h loader.c)