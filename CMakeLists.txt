cmake_minimum_required(VERSION 3.13)
include(CheckLibraryExists)
include(CheckIncludeFiles)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

project(llvm-playground CXX)

find_package(LLVM REQUIRED CONFIG)
#list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
#include(AddLLVM)

message(STATUS "Found LLVM: ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using LLVM libs: ${LLVM_LIBRARY_DIRS}")

find_package(Clang REQUIRED)
find_package(Clang REQUIRED clangTooling libClang)

add_subdirectory(src)
