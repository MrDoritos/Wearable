idf_component_register(
    SRCS "wearable.cpp"
    INCLUDE_DIRS "." "./display" "./ui" "./common"
    PRIV_REQUIRES spi_flash esp_driver_i2c esp_timer
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-narrowing
    -Wno-type-limits
    -Wno-missing-field-initializers
    -fpermissive
)

target_add_binary_data(${COMPONENT_LIB}
    ../assets/dosjpn.bin
    ALIGN_BYTES 4
    EMBED_AS_ARRAY
    SOURCE_DIR ../assets
)

target_add_binary_data(${COMPONENT_LIB}
    ../assets/textures.bin
    ALIGN_BYTES 4
    EMBED_AS_ARRAY
    SOURCE_DIR ../assets
)