cmake_minimum_required(VERSION 3.16)

project(btc LANGUAGES C CXX)

# ------------------------------
# Global settings
# ------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

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
find_package(OpenSSL REQUIRED)
enable_testing()

add_library(btc_core)
target_sources(
  btc_core
  PRIVATE src/Bencode/bencodeValue.cpp src/Bencode/bencodeDecoder.cpp
          src/Bencode/bencodeEncoder.cpp src/Torrent/torrentParser.cpp
          src/Net/httpConnection.cpp src/errors.cpp)

target_include_directories(btc_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(btc_core PUBLIC OpenSSL::SSL OpenSSL::Crypto)

add_executable(btc src/main.cpp)
target_link_libraries(btc PRIVATE btc_core)

add_executable(btc_tests tests/bencodeTest.cpp tests/torrentFileTest.cpp)
target_link_libraries(btc_tests PRIVATE btc_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(btc_tests)

target_compile_options(btc_core PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(btc PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(btc_tests PRIVATE -Wall -Wextra -Wpedantic)
