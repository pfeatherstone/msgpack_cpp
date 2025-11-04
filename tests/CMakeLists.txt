cmake_minimum_required(VERSION 3.13)
project(MsgPackTests)

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
FetchContent_MakeAvailable(Boost)

# Unit tests
add_executable(tests
  main.cpp
  value.cpp
  pack.cpp)
target_compile_features(tests PRIVATE cxx_std_17)
target_compile_options(tests PRIVATE -Wall -Wextra)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(tests  PRIVATE Boost::describe)