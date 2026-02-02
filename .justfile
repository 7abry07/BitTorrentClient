set quiet := true

# run the executable
run:
    ./build/btc

# run the tests
test:
    ./build/btc_tests --gtest_color=yes

# open debugger
debug:
    seergdb 

# debug with valgrind
valg:
    colour-valgrind ./build/btc

# build the project
build:
    make -C build

# reconstruct build files
rebuild type="Debug":
    rm -rf build
    mkdir build
    cmake -B build -DCMAKE_BUILD_TYPE={{ type }}
    just build
