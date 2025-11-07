cmake_minimum_required(VERSION 3.13)
project(MsgPackExamples)

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
FetchContent_MakeAvailable(Boost)

# Examples
function(add_example target_name)
  add_executable(${target_name} ${ARGN})
  target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
  target_compile_options(${target_name} PRIVATE $<${IS_NOT_MSVC}:-Wall -Wextra -Werror>)
  target_link_options(${target_name} PRIVATE $<$<AND:$<CONFIG:RELEASE>,${IS_NOT_MSVC}>:-s>)
  target_link_libraries(${target_name} PRIVATE Boost::describe)
endfunction()

add_example(example1 example1.cpp)
add_example(example2 example2.cpp)
add_example(example3 example3.cpp)
add_example(example4 example4.cpp)