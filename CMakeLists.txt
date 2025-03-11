cmake_minimum_required(VERSION 3.16)
project(Neoluma LANGUAGES CXX)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

# Connecting libraries of LLVM
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Adding executable
add_executable(neoluma main.cpp)

# Linking with LLVM Core 
target_link_libraries(neoluma PRIVATE LLVMCore LLVMSupport)
