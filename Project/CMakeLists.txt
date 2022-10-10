cmake_minimum_required(VERSION 3.13)
project(lab7)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_LIBRARIES "-lws2_32 -lmysql -lgdi32 -lcomctl32")

add_executable(lab7 main.cpp)