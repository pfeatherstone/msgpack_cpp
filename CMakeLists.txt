cmake_minimum_required(VERSION 3.13)
project(Msgpack)

# Dependencies for Unit tests and Bench tests
include(FetchContent)
include(ExternalProject)

# set(MSGPACK_NO_BOOST ON)
set(MSGPACK_USE_BOOST OFF CACHE INTERNAL "Disabling boost in msgpack library")
FetchContent_Declare(
  msgpack
  GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
  GIT_TAG        cpp_master
)
FetchContent_Declare(
    doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        v2.4.11
)
FetchContent_MakeAvailable(msgpack doctest)

# Unit test
add_executable(Test test.cpp)
target_link_libraries(Test PRIVATE msgpack-cxx)
target_include_directories(Test PRIVATE ${doctest_SOURCE_DIR}/doctest)
target_compile_options(Test PRIVATE -Wall -Wextra -Wconversion)