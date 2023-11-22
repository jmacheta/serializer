cmake_minimum_required(VERSION 3.21)

project(serializer LANGUAGES CXX VERSION 0.9.0)

option(CONFIG_ECPP_SERIALIZER_BUILD_TESTS "Forces building test cases" OFF)
option(CONFIG_ECPP_SERIALIZER_BUILD_EXAMPLES "Forces building examples" OFF)

add_library(ecpp_serializer INTERFACE)
add_library(ecpp::serializer ALIAS ecpp_serializer)

include(FetchContent)
FetchContent_Declare(ecpp_endian GIT_REPOSITORY https://github.com/jmacheta/endian.git GIT_TAG main)

FetchContent_MakeAvailable(ecpp_endian)

target_include_directories(ecpp_serializer INTERFACE include)
target_compile_features(ecpp_serializer INTERFACE cxx_std_20)
target_link_libraries(ecpp_serializer INTERFACE ecpp_endian)

if (PROJECT_IS_TOP_LEVEL OR CONFIG_ECPP_SERIALIZER_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif ()

if (PROJECT_IS_TOP_LEVEL OR CONFIG_ECPP_SERIALIZER_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif ()
