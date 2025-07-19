idf_component_register(
    SRCS "user_inputs.cpp" "wearable.cpp" "./common/wbl_func.cpp" "./ui/ui_func.cpp"
    INCLUDE_DIRS "." "./display" "./ui" "./common" "./peripheral"
    PRIV_REQUIRES spi_flash esp_driver_i2c esp_timer esp_driver_gpio
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-narrowing
    -Wno-type-limits
    -Wno-missing-field-initializers
    -Wno-format-truncation
    -Wno-switch
    -fpermissive
)

set(ASSET_DIR "../assets")

get_filename_component(ASSET_FULL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${ASSET_DIR}" REALPATH)

file(GLOB_RECURSE ASSET_FILES "${ASSET_FULL_PATH}/*")
set(GENERATED_ASSET_OBJECTS "")
set(ASSET_OBJ_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets")
file(MAKE_DIRECTORY "${ASSET_OBJ_DIR}")

foreach(asset_file IN LISTS ASSET_FILES)
    file(RELATIVE_PATH asset_relative_path "${ASSET_FULL_PATH}" "${asset_file}")
    set(output_obj_file "${ASSET_OBJ_DIR}/${asset_relative_path}.o")
    #add_custom_command(
    #    OUTPUT ${output_obj_file}
    #    COMMAND cd "${ASSET_FULL_PATH}" && objcopy -I binary -O elf64-x86-64 --rename-section .data=.rodata,alloc,load,readonly,contents "${asset_relative_path}" "${output_obj_file}"
    #    DEPENDS "${asset_file}" 
    #    COMMENT "Generating object file from asset: ${asset_file} -> ${output_obj_file}"
    #)
    target_add_binary_data(${COMPONENT_LIB}
        ${asset_file}
        ALIGN_BYTES 4
        EMBED_AS_ARRAY
        SOURCE_DIR ${ASSET_DIR}
    )

    #list(APPEND GENERATED_ASSET_OBJECTS "${output_obj_file}")
endforeach()

#target_add_binary_data(${COMPONENT_LIB}
#    ../assets/dosjpn.bin
#    ALIGN_BYTES 4
#    EMBED_AS_ARRAY
#    SOURCE_DIR ../assets
#)

#target_add_binary_data(${COMPONENT_LIB}
#    ../assets/textures.bin
#    ALIGN_BYTES 4
#    EMBED_AS_ARRAY
#    SOURCE_DIR ../assets
#)