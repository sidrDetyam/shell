cmake_minimum_required(VERSION 3.22.1)
project(shell C)

set(CMAKE_C_STANDARD 11)

file(GLOB sources src/*.c)

file(GLOB headers include/*.c)

add_executable(shell ${sources} ${headers})
