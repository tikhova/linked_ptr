cmake_minimum_required(VERSION 3.0.0)

project(linked_ptr VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME}
    "main.cpp"
    "linked_ptr.h")
