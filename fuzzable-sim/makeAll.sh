#!/bin/env bash

set -xeuo pipefail

echo Finding vcpkg toolchain file
vcpkg_dir="$(dirname "$(readlink -f "$(which vcpkg)")")"
cmake_toolchain_file="${vcpkg_dir}/scripts/buildsystems/vcpkg.cmake"

echo Building ALE library
if [ ! -d ../build/ ]; then
    mkdir -p ../build/
    cd ../build/
    cmake ../ -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_LIB=OFF \
        -DBUILD_CPP_LIB=ON -DSDL_SUPPORT=ON -DAFL_SUPPORT=ON \
        -DCMAKE_INSTALL_PREFIX=../install/ \
        -DCMAKE_TOOLCHAIN_FILE="$cmake_toolchain_file"
else
    cd ../build/
fi
cmake --build . --target install

echo Building AFL
cd ../AFL
make

echo Building fuzzable simulator
cd ../fuzzable-sim/
if [ ! -d build ]; then
    mkdir -p build
    cd build
    cmake ../ -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_PREFIX_PATH="$PWD/../../install/lib/cmake/" \
          -DCMAKE_TOOLCHAIN_FILE="$cmake_toolchain_file"
else
    cd build
fi
cmake --build . --verbose
