cmake_minimum_required(VERSION 3.13)
project(MsgPackBenchmarks)

# Helpers
set(IS_MSVC       "$<CXX_COMPILER_ID:MSVC>")
set(IS_NOT_MSVC   "$<NOT:${IS_MSVC}>")

# Boost
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

# Examples
add_executable(Bench main.cpp)
target_include_directories(Bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_compile_features(Bench PRIVATE cxx_std_17)
target_compile_options(Bench PRIVATE $<${IS_NOT_MSVC}:-Wall -Wextra -Werror>)
target_link_options(Bench PRIVATE $<$<AND:$<CONFIG:RELEASE>,${IS_NOT_MSVC}>:-s>)
target_link_libraries(Bench PRIVATE Boost::describe msgpack-cxx)