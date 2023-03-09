
set(PLUGIN_NAME NeuralResonatorVST)

set(FORMATS Standalone VST3 AU)

# if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
#     list(APPEND FORMATS LV2)
# endif()

juce_add_plugin(${PLUGIN_NAME}
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME C4DM                           # Specify the name of the plugin's author
    # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
    # NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD FALSE               # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE Diaz               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE NRes                            # A unique four-character plugin id with at least one upper-case character
    FORMATS ${FORMATS}                          # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "NeuralResonator")             # The name of the final executable, which can differ from the target name

# Custom command to compile the ui
# We use the `execute_process` because the `juce_add_binary_data` command
# doesn't support custom commands or generated files
set(MINIFIED_UI_PATH ${CMAKE_SOURCE_DIR}/ui/dist/index.html CACHE INTERNAL "Path to the minified ui")
# check if the ui is already built
# WARNING: any new changes to the ui won't be reflected in the plugin
if (NOT EXISTS ${MINIFIED_UI_PATH})
    # if not, build it
    message(STATUS "Installing dev dependencies for the ui")
    execute_process(
        COMMAND npm i --include=dev
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/ui
        COMMAND_ERROR_IS_FATAL ANY
        COMMAND_ECHO STDOUT
    )
    message(STATUS "Building the ui")
    execute_process(
        COMMAND npm run build
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/ui
        COMMAND_ERROR_IS_FATAL ANY
        COMMAND_ECHO STDOUT
    )
endif()

# Add any resources you need to this target
juce_add_binary_data(
    BinaryData
    SOURCES
    ${MINIFIED_UI_PATH}
)

set_target_properties(
    BinaryData 
    PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
)

juce_generate_juce_header(${PLUGIN_NAME})

target_sources(
    ${PLUGIN_NAME}
    PRIVATE
    PluginEditor.cpp
    PluginProcessor.cpp
    ServerThread.cpp
    BrowserComponent.cpp
    TorchWrapper.cpp
    Filterbank.cpp
    ParameterSyncer.cpp
)

target_compile_definitions(
    ${PLUGIN_NAME}
    PRIVATE
    JUCE_WEB_BROWSER=1  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    # JUCE_DEBUG=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    BROWSER_DEV_SERVER=$<BOOL:${BROWSER_DEV_SERVER}>
)

target_link_libraries(
    ${PLUGIN_NAME}
    PRIVATE
    asio
    juce::juce_gui_extra
    BinaryData
	kac_core
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    # juce::juce_recommended_warning_flags
    juce::juce_audio_utils
    ${TORCH_LIBRARIES}
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(
        ${PLUGIN_NAME}
        PRIVATE
        juce::pkgconfig_JUCE_BROWSER_LINUX_DEPS
    )
    # target_compile_options(
    #     ${PLUGIN_NAME}
    #     PRIVATE
    #     -fsanitize=address
    # )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    get_target_property(
        JUCE_PLUGIN_ARTEFACT_FILE_AU
        ${PLUGIN_NAME}_AU
        JUCE_PLUGIN_ARTEFACT_FILE
    )

    get_target_property(
        JUCE_PLUGIN_ARTEFACT_FILE_VST3
        ${PLUGIN_NAME}_VST3
        JUCE_PLUGIN_ARTEFACT_FILE        
    )

    set(TORCH_LIBS
        ${TORCH_INSTALL_PREFIX}/lib/libtorch.dylib
        ${TORCH_INSTALL_PREFIX}/lib/libtorch_cpu.dylib
        ${TORCH_INSTALL_PREFIX}/lib/libc10.dylib
    )

    add_custom_command(
        TARGET ${PLUGIN_NAME}_AU
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${TORCH_LIBS} "${JUCE_PLUGIN_ARTEFACT_FILE_AU}/Contents/MacOS/"
        COMMENT "Copy Torch Libraries to AU"
        VERBATIM
    )

    add_custom_command(
        TARGET ${PLUGIN_NAME}_VST3
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${TORCH_LIBS} "${JUCE_PLUGIN_ARTEFACT_FILE_VST3}/Contents/MacOS/"
        COMMENT "Copy Torch Libraries to VST3"
        VERBATIM
    )

    juce_enable_copy_plugin_step(${PLUGIN_NAME})
endif()
