cmake_minimum_required(VERSION 3.15)
project(NeuralResonator VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

# Change this option to ON if you want to build the AudioPluginHost for example
# (practical to debug the plugin processor directly from your IDE)
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)

add_subdirectory(third_party/juce)
set(ASIO_PATH third_party/asio)
set(SIMPLE_WEBSOCKET_SERVER_PATH third_party/simple-websocket-server)

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
    )
endif()