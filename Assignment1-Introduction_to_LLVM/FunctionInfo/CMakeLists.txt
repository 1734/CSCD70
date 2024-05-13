cmake_minimum_required(VERSION 3.18.4)
project(FunctionInfo)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LLVM_DIR /home/peach/workspace/llvm-project/install/lib/cmake/llvm)
find_package(LLVM REQUIRED CONFIG)
set(LLVM_VERSION
    "16"
    CACHE STRING "LLVM Version")

# Obtain the build flags from `llvm-config --cxxflags` and forward them to
# variable CMAKE_CXX_FLAGS.
execute_process(
  COMMAND llvm-config --cxxflags
  OUTPUT_VARIABLE LLVM_CXXFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${LLVM_CXXFLAGS}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
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
