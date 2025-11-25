cmake_minimum_required(VERSION 3.13)
project(MyPasses LANGUAGES C CXX)

# 强制使用你期望的 LLVM（如果需要请用 -DLLVM_DIR=... 指定）
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} at ${LLVM_DIR}")

# 让 CMake 能找到 LLVM 的 CMake helper（有些 LLVM 安装需要）
list(APPEND CMAKE_MODULE_PATH "${LLVM_DIR}")

# include AddLLVM，这会定义 llvm_map_components_to_libnames 等 helper
include(AddLLVM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_PASS1 "Build pass1 plugin" ON)
option(BUILD_PASS2 "Build pass2 plugin" ON)
option(BUILD_PASS3 "Build pass3 plugin" ON)

set(PLUGIN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugins")
file(MAKE_DIRECTORY ${PLUGIN_OUTPUT_DIR})

if(BUILD_PASS1)
  add_subdirectory(pass1)
endif()
if(BUILD_PASS2)
  add_subdirectory(pass2)
endif()
if(BUILD_PASS3)
  add_subdirectory(pass3)
endif()
