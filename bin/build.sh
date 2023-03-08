#!/bin/bash
# this is a simple build script that can be used to test or build the project.
# use --test to build the project in debug mode.

# default build commands
cmake_args="-DCMAKE_BUILD_TYPE=Release -DBROWSER_DEV_SERVER=0"
# build commands from arguments
for i in "$@"; do
	case $i in
	--test|-t)
		cmake_args="-DCMAKE_BUILD_TYPE=Debug -DBROWSER_DEV_SERVER=1"
		;;
	*)
		;;
	esac
done

# download libtorch if it doesn't exist already
if ! [[ -d "./third_party/libtorch" ]]
then
    cd ./third_party
		curl -L https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.13.1.zip -o libtorch.zip
		unzip libtorch.zip
		rm -rf libtorch.zip
	cd ../	
fi
# add libtorch to config
cmake_args="${cmake_args} -DCMAKE_PREFIX_PATH=./third_party/libtorch"

# make build dir if it doesn't exist
if ! [[ -d "./build" ]]
then
	mkdir build
fi

# cmake and make
cmake -S . -B ./build $cmake_args
cmake --build build -j