idf_component_register(
    SRCS "wearable.cpp"
    INCLUDE_DIRS "." "./display"
    PRIV_REQUIRES spi_flash
)
