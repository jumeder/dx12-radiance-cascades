find_program(DXC dxc.exe)

function(add_shader FILE TARGET HEADER VARNAME)
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/${HEADER}
        MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/${FILE}
        DEPENDS ${CMAKE_SOURCE_DIR}/shaders/Common.hlsl
        COMMAND ${DXC} /T ${TARGET} /Fh ${CMAKE_SOURCE_DIR}/${HEADER} /Vn ${VARNAME} ${CMAKE_SOURCE_DIR}/${FILE}
        VERBATIM
    )
endfunction()