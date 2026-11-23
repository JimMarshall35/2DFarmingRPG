cmake_minimum_required(VERSION 3.16)

find_package(GTest REQUIRED)


add_executable(
  StardewEngineTest
  DynArrayTests.cpp
  ObjectPoolTests.cpp
  GameFrameworkTests.cpp
  SharedPtrTests.cpp
  StringHashMapTests.cpp
  GameFrameworkEventTests.cpp
  TSQTests.cpp
  main.cpp
)

set_property(TARGET StardewEngineTest PROPERTY CXX_STANDARD 17)

if(WIN32)
target_link_libraries(
  StardewEngineTest
  PRIVATE
  gtest::gtest
)
else()
target_link_libraries(
  StardewEngineTest
  PRIVATE
  GTest::gtest_main
)
endif()


if(WIN32)
target_link_libraries(
  StardewEngineTest
  PRIVATE
  gtest::gtest
)
else()
target_link_libraries(
  StardewEngineTest
  PRIVATE
  GTest::gtest_main
)
include(GoogleTest)
endif()


