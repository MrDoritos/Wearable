cmake_minimum_required(VERSION 3.10)

project(Emulator)

set(ASSET_DIR "../../assets")

get_filename_component(ASSET_FULL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${ASSET_DIR}" REALPATH)

file(GLOB_RECURSE ASSET_FILES "${ASSET_FULL_PATH}/*")
set(GENERATED_ASSET_OBJECTS "")
set(ASSET_OBJ_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets")
file(MAKE_DIRECTORY "${ASSET_OBJ_DIR}")

foreach(asset_file IN LISTS ASSET_FILES)
    file(RELATIVE_PATH asset_relative_path "${ASSET_FULL_PATH}" "${asset_file}")
    set(output_obj_file "${ASSET_OBJ_DIR}/${asset_relative_path}.o")
    add_custom_command(
        OUTPUT ${output_obj_file}
        COMMAND cd "${ASSET_FULL_PATH}" && objcopy -I binary -O elf64-x86-64 --rename-section .data=.rodata,alloc,load,readonly,contents "${asset_relative_path}" "${output_obj_file}"
        DEPENDS "${asset_file}" 
        COMMENT "Generating object file from asset: ${asset_file} -> ${output_obj_file}"
    )

    list(APPEND GENERATED_ASSET_OBJECTS "${output_obj_file}")
endforeach()

add_library(lib
    ../../console/console.ansi.cpp
    ../wearable.cpp
    ${GENERATED_ASSET_OBJECTS}
)

target_compile_options(lib PUBLIC
    -g
)

add_executable(Emulator
    emulator.cpp
)

add_executable(Tests
    tests/layout.cpp
)

target_include_directories(lib PUBLIC
    ../ui
    ../../console
    ./stubs
    .
)

target_link_libraries(Emulator
    lib
)

target_link_libraries(Tests
    lib
)

target_compile_options(Emulator PUBLIC
    -g
    -z noexecstack
)

target_compile_options(Tests PUBLIC
    -g
    -z noexecstack
    -Wfatal-errors
)