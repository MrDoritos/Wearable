idf_component_register(
    SRCS "user_inputs.cpp" "wearable.cpp" "./common/wbl_func.cpp" "./ui/ui_func.cpp" "./ui/sprites.cpp"
    INCLUDE_DIRS "." "./display" "./ui" "./common" "./peripheral" "./log"
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

file(GLOB_RECURSE ASSET_FILES "${ASSET_FULL_PATH}/*.bin")
set(GENERATED_ASSET_OBJECTS "")
set(ASSET_OBJ_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets")
file(MAKE_DIRECTORY "${ASSET_OBJ_DIR}")

foreach(asset_file IN LISTS ASSET_FILES)
    file(RELATIVE_PATH asset_relative_path "${ASSET_FULL_PATH}" "${asset_file}")
    target_add_binary_data(${COMPONENT_LIB}
        ${asset_file}
        ALIGN_BYTES 4
        EMBED_AS_ARRAY
        SOURCE_DIR ${ASSET_DIR}
    )
endforeach()