cmake_minimum_required(VERSION 3.15)
project(NeuralResonator VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

# verbose
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Change this option to ON if you want to build the AudioPluginHost for example
# (practical to debug the plugin processor directly from your IDE)
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(ASIO_STANDALONE "Use standalone ASIO" ON)
option(BROWSER_DEV_SERVER "Use browser dev server" OFF)
option(CMAKE_EXPORT_COMPILE_COMMANDS "Generate compile_commands.json" ON)

# Dependencies

# Torch
# list(APPEND CMAKE_PREFIX_PATH "/Users/diaz/mambaforge/envs/modal/lib/python3.10/site-packages/torch")
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# kac core
add_subdirectory(third_party/kac_core)

# JUCE
add_subdirectory(third_party/juce)

# ASIO
set(ASIO_SOURCE_DIR third_party/asio)
add_library(asio INTERFACE)
target_include_directories(
    asio 
    INTERFACE
    ${ASIO_SOURCE_DIR}/asio/include)
target_compile_definitions(
    asio
    INTERFACE
    ASIO_STANDALONE
    # ASIO_NO_DEPRECATED
)

# Silent cpp warnings
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_compile_options(/W0)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_compile_options(
        -Wno-unused-parameter
        -Wno-redundant-decls
        -Wno-zero-as-null-pointer-constant
        -Wno-shadow
        -Wno-deprecated-declarations
    )
    set(CMAKE_MACOSX_RPATH TRUE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
    set(CMAKE_INSTALL_RPATH "@loader_path")
    # set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
endif()

add_subdirectory(NeuralResonatorVST)
