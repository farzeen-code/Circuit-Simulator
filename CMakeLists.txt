cmake_minimum_required(VERSION 3.16)
project(CircuitSimulator VERSION 1.0 LANGUAGES CXX)

# Enforece Modern C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.cpp
)

add_executable(CircuitSimulator ${SOURCES})

# Include headers directory
target_include_directories(CircuitSimulator PRIVATE src)