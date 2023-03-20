
function(get_torch_libs libs)

    if (NOT TORCH_INSTALL_PREFIX)
        message(FATAL_ERROR "TORCH_INSTALL_PREFIX is not set")
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

        set(
            TORCH_LIBS
            ${TORCH_INSTALL_PREFIX}/lib/libtorch.dylib
            ${TORCH_INSTALL_PREFIX}/lib/libtorch_cpu.dylib
            ${TORCH_INSTALL_PREFIX}/lib/libc10.dylib
        )

        # if the architecture is arm64, we need to add the libiomp5 library
        if (CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
            list(APPEND TORCH_LIBS ${TORCH_INSTALL_PREFIX}/lib/libiomp5.dylib)
        endif()

    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

        set(
            TORCH_LIBS
            ${TORCH_INSTALL_PREFIX}/lib/libtorch.so
            ${TORCH_INSTALL_PREFIX}/lib/libtorch_cpu.so
            ${TORCH_INSTALL_PREFIX}/lib/libc10.so
            ${TORCH_INSTALL_PREFIX}/lib/libgomp-52f2fd74.so.1
        )
    else()
        message(FATAL_ERROR "Unsupported system: ${CMAKE_SYSTEM_NAME}")
    endif()

    set(${libs} ${TORCH_LIBS} PARENT_SCOPE)

endfunction()

# copy the torch libraries to the build directory
function(copy_torch_libs target)
    
    get_torch_libs(TORCH_LIBS)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${TORCH_LIBS} "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Copy Torch Libraries to ${target}"
    )

endfunction()