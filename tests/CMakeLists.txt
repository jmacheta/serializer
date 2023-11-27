include(FetchContent)

FetchContent_Declare(
  googletest URL https://github.com/google/googletest/archive/main.zip FIND_PACKAGE_ARGS NAMES
                 GTest
)

FetchContent_MakeAvailable(googletest)

add_executable(
  serializer_ut composite_serialization.cpp containers_serialization.cpp
                fundamental_types_serialization.cpp
)

target_link_libraries(serializer_ut PRIVATE ecpp::serializer GTest::gtest_main GTest::gmock)
target_link_options(serializer_ut PUBLIC -pthread)
target_compile_options(
  serializer_ut
  PRIVATE -Wall
          -Wextra
          -pedantic
          -Wconversion
          -Winit-self
          -Wcast-qual
          -Wcast-align
          -Wpacked
          -Wstrict-overflow=5
          -Wstrict-aliasing
          -fstrict-aliasing
          -Wredundant-decls
          -Wmissing-declarations
          -Wdisabled-optimization
          -Wold-style-cast
)

include(GoogleTest)
gtest_discover_tests(serializer_ut)
