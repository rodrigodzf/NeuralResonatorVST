
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
    NEEDS_MIDI_INPUT TRUE                       # Does the plugin need midi input?
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

# Use or not the SIMPLE UI
if (USE_SIMPLE_UI)
    message(STATUS "Using the simple UI")
    target_compile_definitions(
        ${PLUGIN_NAME}
        PUBLIC
        SIMPLE_UI=1
    )
endif()

# Add any resources you need to this target
# juce_add_binary_data(
#     BinaryData
#     SOURCES
#     ${MINIFIED_UI_PATH}
# )

# set_target_properties(
#     BinaryData 
#     PROPERTIES
#     POSITION_INDEPENDENT_CODE TRUE
# )

juce_generate_juce_header(${PLUGIN_NAME})

target_sources(
    ${PLUGIN_NAME}
    PRIVATE
    PluginEditor.cpp
    PluginProcessor.cpp
    TorchWrapper.cpp
    Filterbank.cpp
)

if (USE_SIMPLE_UI)
    target_sources(
        ${PLUGIN_NAME}
        PRIVATE
        DragableVertex.cpp
        FallbackUI.cpp
    )
else()
    target_sources(
        ${PLUGIN_NAME}
        PRIVATE
        ServerThread.cpp
        BrowserComponent.cpp
        ParameterSyncer.cpp
    )
endif()

target_compile_definitions(
    ${PLUGIN_NAME}
    PRIVATE
    JUCE_WEB_BROWSER=$<NOT:$<BOOL:${USE_SIMPLE_UI}>>  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0  # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    # JUCE_DEBUG=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    BROWSER_DEV_SERVER=$<BOOL:${BROWSER_DEV_SERVER}>
)

target_link_libraries(
    ${PLUGIN_NAME}
    PUBLIC
    asio
    juce::juce_gui_extra
	kac_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    # juce::juce_recommended_warning_flags
    juce::juce_audio_utils
    ${TORCH_LIBRARIES}
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (NOT USE_SIMPLE_UI)
        target_link_libraries(
            ${PLUGIN_NAME}
            PUBLIC
            juce::pkgconfig_JUCE_BROWSER_LINUX_DEPS
        )
    endif()
    # target_compile_options(
    #     ${PLUGIN_NAME}
    #     PRIVATE
    #     -fsanitize=address
    # )
endif()


set(
    PRETRAINED_MODELS_PATH
    ${CMAKE_SOURCE_DIR}/pretrained/encoder.pt
    ${CMAKE_SOURCE_DIR}/pretrained/model_wrap.pt
    CACHE INTERNAL "Path to the pretrained models"
)

get_target_property(active_targets ${PLUGIN_NAME} JUCE_ACTIVE_PLUGIN_TARGETS)
foreach( sub_target IN LISTS active_targets )
    
    message(STATUS "Adding libraries and models to ${sub_target}")

    get_target_property(
        BUNDLE_PATH
        ${sub_target}
        JUCE_PLUGIN_ARTEFACT_FILE
    )

    copy_torch_libs(${sub_target})
    
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

        # Copy pretrained models to the plugin bundle
        target_sources(${sub_target} PRIVATE ${PRETRAINED_MODELS_PATH})
        set_source_files_properties(
            ${PRETRAINED_MODELS_PATH}
            PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )

        if (NOT USE_SIMPLE_UI)
            # Copy the minified ui to the plugin bundle
            target_sources(${sub_target} PRIVATE ${MINIFIED_UI_PATH})
            set_source_files_properties(
                ${MINIFIED_UI_PATH}
                PROPERTIES
                MACOSX_PACKAGE_LOCATION "Resources"
            )
        endif()

    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # Copy pretrained models to the plugin bundle
        add_custom_command(
            TARGET ${sub_target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            ARGS -E copy ${PRETRAINED_MODELS_PATH} "$<TARGET_FILE_DIR:${sub_target}>"
            COMMENT "Copy models to ${sub_target}"
            VERBATIM
        )
        if (NOT USE_SIMPLE_UI)
            # Copy the minified ui to the plugin bundle
            add_custom_command(
                TARGET ${sub_target}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND}
                ARGS -E copy ${MINIFIED_UI_PATH} "$<TARGET_FILE_DIR:${sub_target}>"
                COMMENT "Copy minified ui to ${sub_target}"
                VERBATIM
            )
        endif()
    endif()
endforeach()

juce_enable_copy_plugin_step(${PLUGIN_NAME})
