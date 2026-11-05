cmake_minimum_required(VERSION 3.20)

project(itmo_boolean_expressions LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BOOLEAN_EXPRESSIONS_USE_SYSTEM_GTEST
       "Use an installed GoogleTest package instead of FetchContent"
       OFF)

add_library(boolean_expressions
  src/analyzer.cc
)

target_include_directories(boolean_expressions
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
  target_compile_options(boolean_expressions PRIVATE /W4 /permissive-)
else()
  target_compile_options(boolean_expressions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(boolean_expressions_example
  examples/main.cc
)

target_link_libraries(boolean_expressions_example
  PRIVATE
    boolean_expressions
)

include(CTest)

if(BUILD_TESTING)
  if(BOOLEAN_EXPRESSIONS_USE_SYSTEM_GTEST)
    find_package(GTest CONFIG REQUIRED)
  else()
    include(FetchContent)

    # Keep the parent project's MSVC runtime choice on Windows.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)
  endif()

  add_executable(boolean_expressions_tests
    tests/analyzer_test.cc
  )

  target_link_libraries(boolean_expressions_tests
    PRIVATE
      boolean_expressions
      GTest::gtest_main
  )

  include(GoogleTest)
  gtest_discover_tests(boolean_expressions_tests)
endif()
