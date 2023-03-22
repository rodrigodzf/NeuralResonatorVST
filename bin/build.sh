#!/bin/bash
# this is a simple build script that can be used to test or build the project.
# use --test to build the project in debug mode.

# default build commands
cmake_args="-DCMAKE_BUILD_TYPE=Release -DBROWSER_DEV_SERVER=0"
# build commands from arguments
for i in "$@"; do
    case $i in
    --test | -t)
        cmake_args="$cmake_args -DCMAKE_BUILD_TYPE=Debug -DBROWSER_DEV_SERVER=1"
        ;;
    --simple_ui | -s)
        cmake_args="$cmake_args -DSIMPLE_UI=1"
        ;;
    *) ;;
    esac
done

uname=$(uname)
arch=$(uname -m)

# for each platform, we need to download a different version of libtorch

# linux
if [[ "$uname" == "Linux" ]]; then
    # download libtorch if it doesn't exist already
    if ! [[ -d "./third_party/libtorch" ]]; then
        cd ./third_party
        curl -L https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.0.0%2Bcpu.zip -o libtorch.zip
        unzip libtorch.zip
        rm -rf libtorch.zip
        cd ../
    fi
fi

# macos (intel)

if [[ "$uname" == "Darwin" ]] && [[ "$arch" == "x86_64" ]]; then
    # download libtorch if it doesn't exist already
    if ! [[ -d "./third_party/libtorch" ]]; then
        cd ./third_party
        curl -L https://download.pytorch.org/libtorch/cpu/libtorch-macos-2.0.0.zip -o libtorch.zip
        unzip libtorch.zip
        rm -rf libtorch.zip
        cd ../
    fi
fi

# macos (arm)

if [[ "$uname" == "Darwin" ]] && [[ "$arch" == "arm64" ]]; then
    # download libtorch if it doesn't exist already
    if ! [[ -d "./third_party/libtorch" ]]; then
        cd ./third_party
        curl -L https://anaconda.org/pytorch/pytorch/2.0.0/download/osx-arm64/pytorch-2.0.0-py3.9_0.tar.bz2 -o pytorch.tar.bz2
        mkdir pytorch
        tar -xvf pytorch.tar.bz2 -C pytorch
        cp -r pytorch/lib/python3.9/site-packages/torch libtorch
        rm -rf pytorch pytorch.tar.bz2
        cd ../
    fi
fi

# add libtorch to config
cmake_args="${cmake_args} -DCMAKE_PREFIX_PATH=./third_party/libtorch"

# make build dir if it doesn't exist
if ! [[ -d "./build" ]]; then
    mkdir build
fi

# print the build commands
echo "cmake -S . -B build $cmake_args"
# cmake and make
cmake -S . -B build $cmake_args
cmake --build build -j
