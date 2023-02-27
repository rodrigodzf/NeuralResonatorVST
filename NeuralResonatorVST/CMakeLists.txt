
set(PLUGIN_NAME NeuralResonatorVST)

juce_add_plugin(${PLUGIN_NAME}
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    # COMPANY_NAME ...                          # Specify the name of the plugin's author
    # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
    # NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE Reju               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Dem0                            # A unique four-character plugin id with at least one upper-case character
    FORMATS Standalone                          # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "NeuralResonator")             # The name of the final executable, which can differ from the target name

# Add any resources you need to this target
# juce_add_binary_data(
#     ${PLUGIN_NAME}_BINARY_DATA
#     SOURCES 
#     config.json
# )


juce_generate_juce_header(${PLUGIN_NAME})

target_sources(
    ${PLUGIN_NAME}
    PRIVATE
    PluginEditor.cpp
    PluginProcessor.cpp
    ServerThread.cpp
    BrowserComponent.cpp
    TorchWrapper.cpp
)

target_compile_definitions(
    ${PLUGIN_NAME}
    PRIVATE
    JUCE_WEB_BROWSER=1  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_DEBUG=1
    JUCE_VST3_CAN_REPLACE_VST2=0
)

if (ASIO_STANDALONE)
    # for the wss server we need openssl
    find_package(OpenSSL REQUIRED)
    message(STATUS "OpenSSL found at ${OPENSSL_INCLUDE_DIR}")

    target_compile_definitions(
        ${PLUGIN_NAME}
        PRIVATE
        USE_STANDALONE_ASIO
    )
    find_path(ASIO_PATH asio.hpp REQUIRED)
    cmake_path(ABSOLUTE_PATH ASIO_PATH BASE_DIRECTORY ${PROJECT_SOURCE_DIR})
    if(NOT ASIO_PATH)
        message(FATAL_ERROR "Standalone Asio not found")
    else()
        message(STATUS "Standalone Asio found at ${ASIO_PATH}")
        target_include_directories(
            ${PLUGIN_NAME}
            PRIVATE
            ${ASIO_PATH}/include
        )
    endif()
endif()

find_path(SIMPLE_WEBSOCKET_SERVER_PATH server_ws.hpp REQUIRED)
if (NOT SIMPLE_WEBSOCKET_SERVER_PATH)
    message(FATAL_ERROR "Simple Websocket Server not found")
else()
    cmake_path(ABSOLUTE_PATH SIMPLE_WEBSOCKET_SERVER_PATH BASE_DIRECTORY ${PROJECT_SOURCE_DIR})
    message(STATUS "Simple Websocket Server found at ${SIMPLE_WEBSOCKET_SERVER_PATH}")

    target_include_directories(
        ${PLUGIN_NAME}
        PUBLIC
        ${SIMPLE_WEBSOCKET_SERVER_PATH}
    )
endif()

target_include_directories(
    ${PLUGIN_NAME}
    INTERFACE
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(
    ${PLUGIN_NAME}
    PRIVATE
    OpenSSL::SSL
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    # juce::juce_recommended_warning_flags
    juce::juce_audio_utils
    torchwrapper
)

if (LINUX)
    target_link_libraries(
        ${PLUGIN_NAME}
        PRIVATE
        juce::pkgconfig_JUCE_BROWSER_LINUX_DEPS
    )
endif()