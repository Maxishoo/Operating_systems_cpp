cmake_minimum_required(VERSION 3.10)
project(os_labs)
set(CUR_PR "lab2")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${CUR_PR}_exe main.cpp)