set(EXE_NAME "NeuralResonatorTest")

add_executable(${EXE_NAME})

target_sources(${EXE_NAME} PRIVATE ConsolePlugin.cpp)

target_include_directories(${EXE_NAME} PRIVATE ../)

target_link_libraries(
    ${EXE_NAME}
    PRIVATE
    NeuralResonatorVST
)

get_torch_libs(TORCH_LIBS)

set(
    PRETRAINED_MODELS_PATH
    ${CMAKE_SOURCE_DIR}/pretrained/encoder.pt
    ${CMAKE_SOURCE_DIR}/pretrained/model_wrap.pt
    CACHE INTERNAL "Path to the pretrained models"
)

# Copy the torch libraries to the plugin bundle
copy_torch_libs(${EXE_NAME})

# Copy pretrained models to the plugin bundle
add_custom_command(
    TARGET ${EXE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy ${PRETRAINED_MODELS_PATH} "$<TARGET_FILE_DIR:${EXE_NAME}>"
    COMMENT "Copy models to ${EXE_NAME}"
    VERBATIM
)

# Copy the minified ui to the plugin bundle
add_custom_command(
    TARGET ${EXE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy ${MINIFIED_UI_PATH} "$<TARGET_FILE_DIR:${EXE_NAME}>"
    COMMENT "Copy minified ui to ${EXE_NAME}"
    VERBATIM
)
