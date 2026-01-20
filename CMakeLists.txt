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
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v6.1.9)
FetchContent_MakeAvailable(ftxui)

enable_testing()

# ------------------------------
# Production executable
# ------------------------------
add_executable(
  ${PROJECT_NAME}
  src/main.cpp src/Bencode/bencodeValue.cpp src/Bencode/bencodeDecoder.cpp
  src/Bencode/bencodeEncoder.cpp src/torrent.cpp)

# ------------------------------
# Tests executable
# ------------------------------
add_executable(
  btc_tests
  tests/bencodeTest.cpp tests/torrentFileTest.cpp src/Bencode/bencodeValue.cpp
  src/Bencode/bencodeDecoder.cpp src/Bencode/bencodeEncoder.cpp src/torrent.cpp)

find_package(OpenSSL REQUIRED)
target_link_libraries(
  btc
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(
  btc_tests
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component OpenSSL::SSL OpenSSL::Crypto GTest::gtest_main)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(btc_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(btc_tests)

# ------------------------------
# Compiler warnings
# ------------------------------
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
  target_compile_options(btc_tests PRIVATE /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(btc_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
