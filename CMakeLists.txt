cmake_minimum_required(VERSION 3.15)
project(NeuralResonator VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

# Change this option to ON if you want to build the AudioPluginHost for example
# (practical to debug the plugin processor directly from your IDE)
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)

add_subdirectory(third_party/juce)
set(ASIO_PATH third_party/asio)
set(SIMPLE_WEBSOCKET_SERVER_PATH third_party/simple-websocket-server)

# Torch
list(APPEND CMAKE_PREFIX_PATH "/media/fast/libs/libtorch-cxx11-abi-shared-with-deps-1.13.0+cu117/libtorch/share/cmake/Torch")
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
add_subdirectory(third_party/torchplugins/src/common/torchwrapper)

add_subdirectory(NeuralResonatorVST)

# Silent cpp warnings
if(MSVC)
    add_compile_options(/W0)
else()
    add_compile_options(
        -Wno-unused-parameter
        -Wno-redundant-decls
        -Wno-zero-as-null-pointer-constant
        -Wno-shadow
        -Wno-deprecated-declarations
    )
endif()