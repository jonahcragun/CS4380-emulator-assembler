
cmake_minimum_required(VERSION 3.14)
project(Emulator_Assembler)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(
    emu4380
    src/main.cpp src/emu4380.cpp include/emu4380.h src/cache.cpp include/cache.h
)

enable_testing()

add_executable(
  runTests
  tests/tests.cpp include/emu4380.h src/emu4380.cpp src/cache.cpp include/cache.h
)
target_link_libraries(
  runTests
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(runTests)

