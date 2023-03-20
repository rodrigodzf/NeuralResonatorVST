set(EXE_NAME "NeuralResonatorTest")

juce_add_console_app(${EXE_NAME})

target_sources(${EXE_NAME} PRIVATE ConsolePlugin.cpp)

target_include_directories(${EXE_NAME} PRIVATE ../)

target_link_libraries(
    ${EXE_NAME}
    PRIVATE
    asio
    juce::juce_gui_extra
    # BinaryData
	kac_core
    PUBLIC
    NeuralResonatorVST
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    set(
        TORCH_LIBS
        ${TORCH_INSTALL_PREFIX}/lib/libtorch.dylib
        ${TORCH_INSTALL_PREFIX}/lib/libtorch_cpu.dylib
        ${TORCH_INSTALL_PREFIX}/lib/libc10.dylib
    )

    # force set the architecture variable to the current architecture
    # this is needed because the CMAKE_OSX_ARCHITECTURES variable is not set
    if (NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "" FORCE)
    endif()

    # if the architecture is arm64, we need to add the libiomp5 library
    if (CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        list(APPEND TORCH_LIBS ${TORCH_INSTALL_PREFIX}/lib/libiomp5.dylib)
    endif()

    set(
        PRETRAINED_MODELS_PATH
        ${CMAKE_SOURCE_DIR}/pretrained/encoder.pt
        ${CMAKE_SOURCE_DIR}/pretrained/model_wrap.pt
        CACHE INTERNAL "Path to the pretrained models"
    )

    get_target_property(
        RUNTIME_PATH
        ${EXE_NAME}
        RUNTIME_OUTPUT_DIRECTORY
    )

    # Copy the torch libraries to the plugin bundle
    add_custom_command(
        TARGET ${EXE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${TORCH_LIBS} "${RUNTIME_PATH}"
        COMMENT "Copy Torch Libraries to ${RUNTIME_PATH}"
        VERBATIM
    )

    # Copy pretrained models to the plugin bundle
    add_custom_command(
        TARGET ${EXE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${PRETRAINED_MODELS_PATH} "${RUNTIME_PATH}"
        COMMENT "Copy Torch Libraries to ${RUNTIME_PATH}"
        VERBATIM
    )

    # Copy the minified ui to the plugin bundle
    add_custom_command(
        TARGET ${EXE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${MINIFIED_UI_PATH} "${RUNTIME_PATH}"
        COMMENT "Copy Torch Libraries to ${RUNTIME_PATH}"
        VERBATIM
    )
endif()