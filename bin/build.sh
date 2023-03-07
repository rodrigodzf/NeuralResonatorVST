#!/bin/bash

# download libtorch if it doesn't exist already
if ! [[ -d "./third_party/libtorch" ]]
then
    cd ./third_party
		curl -L https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.13.1.zip -o libtorch.zip
		unzip libtorch.zip
		rm -rf libtorch.zip
	cd ../
fi

# make build dir if it doesn't exist
if ! [[ -d "./build" ]]
then
	mkdir build
fi

# cmake and make
cmake -S . -B ./build -DCMAKE_PREFIX_PATH=./third_party/libtorch -DCMAKE_BUILD_TYPE=Release -DBROWSER_DEV_SERVER=1
cd build
	make -j
cd ../