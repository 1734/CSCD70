cmake_minimum_required(VERSION 3.18.4)
project(PassManager)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(MY_LLVM_INSTALL_DIR /home/peach/workspace/llvm-project/install_debug)
set(LLVM_DIR ${MY_LLVM_INSTALL_DIR}/lib/cmake/llvm)

set(LLVM_VERSION
    "16"
    CACHE STRING "LLVM Version")

execute_process(
  COMMAND llvm-config --cxxflags
  OUTPUT_VARIABLE LLVM_CXXFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${LLVM_CXXFLAGS}")
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
  # set(CMAKE_CXX_CLANG_TIDY
  #     "clang-tidy;--extra-arg-before=-stdlib=libc++")
  # message(STATUS "Enabling clang-tidy")

  execute_process(
    COMMAND llvm-config --includedir
    OUTPUT_VARIABLE LLVM_INCLUDEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -isystem ${LLVM_INCLUDEDIR} -Wall -Wextra -Werror")
endif()

add_subdirectory(lib)

include(CTest)
enable_testing()
add_subdirectory(test)
