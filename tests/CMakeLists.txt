cmake_minimum_required(VERSION 3.13)
project(MsgPackTests)

# Helpers
set(IS_MSVC       "$<CXX_COMPILER_ID:MSVC>")
set(IS_NOT_MSVC   "$<NOT:${IS_MSVC}>")

# Deps
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

# Unit tests
add_executable(tests
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/msgpack.cpp
  main.cpp
  value.cpp
  pack.cpp)
target_compile_features(tests PRIVATE cxx_std_17)
target_compile_options(tests PRIVATE $<${IS_NOT_MSVC}:-Wall -Wextra -Wpedantic -Werror>)
target_compile_options(tests PRIVATE $<${IS_MSVC}:/Wall /WX>)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(tests  PRIVATE Boost::describe PRIVATE msgpack-cxx)