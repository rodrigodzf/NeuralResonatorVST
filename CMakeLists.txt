cmake_minimum_required(VERSION 3.15)
project(NeuralResonator VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

# verbose
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Change this option to ON if you want to build the AudioPluginHost for example
# (practical to debug the plugin processor directly from your IDE)
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(ASIO_STANDALONE "Use standalone ASIO" ON)
option(BROWSER_DEV_SERVER "Use browser dev server" OFF)

add_subdirectory(third_party/juce)
set(ASIO_PATH third_party/asio/asio)
set(SIMPLE_WEBSOCKET_SERVER_PATH third_party/simple-websocket-server)

# Torch
# list(APPEND CMAKE_PREFIX_PATH "/Users/diaz/mambaforge/envs/modal/lib/python3.10/site-packages/torch")
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
add_subdirectory(third_party/torchplugins/src/common/torchwrapper)

# Silent cpp warnings
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_compile_options(/W0)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_compile_options(
        -Wno-unused-parameter
        -Wno-redundant-decls
        -Wno-zero-as-null-pointer-constant
        -Wno-shadow
        -Wno-deprecated-declarations
    )
endif()

add_subdirectory(NeuralResonatorVST)
