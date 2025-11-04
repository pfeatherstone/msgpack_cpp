cmake_minimum_required(VERSION 3.13)
project(Msgpack)

# Dependencies for Unit tests and Bench tests
include(FetchContent)

set(BOOST_INCLUDE_LIBRARIES describe)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
  Boost
  URL "https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-cmake.tar.xz"
  URL_HASH MD5=537da0e22b31b8b7185cc44fdde70458
  DOWNLOAD_EXTRACT_TIMESTAMP true
)

set(MSGPACK_USE_BOOST OFF CACHE INTERNAL "Disabling boost in msgpack library")
FetchContent_Declare(
  msgpack
  GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
  GIT_TAG        cpp_master
)

FetchContent_MakeAvailable(Boost msgpack)

# Unit test
add_executable(Test test.cpp)
target_compile_features(Test PRIVATE cxx_std_17)
target_compile_options(Test PRIVATE -Wall -Wextra)
target_include_directories(Test 
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(Test 
  PRIVATE Boost::describe Boost::unit_test_framework
  PRIVATE msgpack-cxx)

# Example
add_executable(Example example.cpp)
target_compile_features(Example PRIVATE cxx_std_17)
target_compile_options(Example PRIVATE -Wall -Wextra)
target_include_directories(Example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(Example PRIVATE Boost::describe)