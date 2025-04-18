cmake_minimum_required(VERSION 3.9)
project(ThreadAnalysisTool)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_STANDARD 17)

add_executable(CallGraphExtractor CallGraphExtractor.cpp)
target_link_libraries(CallGraphExtractor
  clangTooling
  clangBasic
  clangASTMatchers
)

add_executable(ThreadDetector ThreadDetector.cpp)
target_link_libraries(ThreadDetector
  clangTooling
  clangBasic
  clangASTMatchers
)
