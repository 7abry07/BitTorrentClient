
cmake_minimum_required(VERSION 3.16)

project(btc)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ------------------------------
# Production executable
# ------------------------------
add_executable(${PROJECT_NAME} src/main.cpp src/bencode.cpp)

# ------------------------------
# Tests executable
# ------------------------------
add_executable(test_bencode tests/bencodeTest.cpp src/bencode.cpp)
target_link_libraries(test_bencode GTest::gtest_main)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)
target_include_directories(test_bencode
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(test_bencode)

# ------------------------------
# Compiler warnings
# ------------------------------
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_options(test_bencode PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(test_bencode PRIVATE -Wall -Wextra -Wpedantic)
endif()

