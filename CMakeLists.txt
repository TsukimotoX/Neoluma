cmake_minimum_required(VERSION 3.16)
project(Neoluma LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_library(NeolumaLib ${SRC_FILES})
add_executable(neoluma ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(neoluma PRIVATE LLVMCore LLVMSupport NeolumaLib)

