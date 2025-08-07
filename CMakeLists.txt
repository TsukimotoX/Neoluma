cmake_minimum_required(VERSION 4.0)

# Neoluma, a programming language
project(Neoluma)

# Set up C++ version
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set up LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

# Includes everything in src/
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Compile the language and Command-Line-Interface
file(GLOB_RECURSE CORE_FILES src/Core/*.cpp src/Core/*.hpp)
file(GLOB_RECURSE CLI_FILES src/CLI/*.cpp src/CLI/*.hpp)
file(GLOB_RECURSE LIB_FILES src/Libraries/*.cpp src/Libraries/*.hpp)

# Compile a core library
add_library(NeolumaCore ${CORE_FILES})
add_library(NeolumaCLI ${CLI_FILES})
add_library(NeolumaLibs ${LIB_FILES}) # Self-written or used libraries.

# Compile the language
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/.build/.executables)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/.build/.libs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/.build/.libs)
add_executable(Neoluma ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/HelperFunctions.hpp)

# Link all libraries
target_link_libraries(Neoluma PRIVATE LLVMCore LLVMSupport LLVMIRReader NeolumaCore NeolumaCLI NeolumaLibs)