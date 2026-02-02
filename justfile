# runs the executable
run:
    ./build/btc

# runs the tests
test:
    ./build/btc_tests --gtest_color=yes

# opens debugger
debug:
    seergdb 

# builds the project
build:
    make -C build

# reconstructs build files
rebuild type="Debug":
    rm -rf build
    mkdir build
    cmake -B build -DCMAKE_BUILD_TYPE={{ type }}
    just build
