cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

option(ENABLE_EXTRA_CHECKS "Enabel Additional Warnings and Sanitizer Checks" false)
if (ENABLE_EXTRA_CHECKS)
  message("extra checks are on")
  set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif()

# project(test_cexpr_basic_string LANGUAGES CXX)

# Add libraries
find_package(Catch2 REQUIRED)
add_library(read lib/read.cpp)
target_include_directories(read PUBLIC include)

add_library(eval lib/eval.cpp)
target_include_directories(eval PUBLIC include)

add_executable(lisp app/main.cpp)
target_include_directories(lisp PUBLIC include)
target_link_libraries(lisp read eval Catch2::Catch2)


add_executable(test_read app/tests_read.cpp)
target_include_directories(test_read PUBLIC include)
target_link_libraries(test_read read Catch2::Catch2)

add_executable(test_eval app/tests_eval.cpp)
target_include_directories(test_eval PUBLIC include)
target_link_libraries(test_eval eval read Catch2::Catch2)

