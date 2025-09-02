cmake_minimum_required(VERSION 3.15)

project(acc VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(acc main.cpp)
target_compile_options(acc PUBLIC -fsanitize=address)
target_link_options(acc PUBLIC -fsanitize=address)
