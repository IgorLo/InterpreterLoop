cmake_minimum_required(VERSION 3.10)
project(InterpreterLoop C)

set(CMAKE_C_STANDARD 99)

add_executable(InterpreterLoop main.c analyzer.c analyzer.h constants.h)