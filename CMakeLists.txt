cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
#set(PICO_DEOPTIMIZED_DEBUG 1)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/Users/timax/Development/pi_pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(klik C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add local files
aux_source_directory(. PROJECT_SOURCES)

aux_source_directory(./libs/picow_tls_client PICO_TLS_CLIENT)

# Add executable. Default name is the project name
add_executable(klik klik.c ${PROJECT_SOURCES} ${PICO_TLS_CLIENT})

target_include_directories(klik PRIVATE
    ./libs/picow_tls_client
    )

pico_set_program_name(klik "klik")
pico_set_program_version(klik "1.2")

pico_enable_stdio_uart(klik 1)
pico_enable_stdio_usb(klik 1)

# Add the standard library to the build
target_link_libraries(klik pico_stdlib hardware_pwm hardware_flash hardware_sync pico_cyw43_arch_lwip_poll pico_lwip_mbedtls pico_mbedtls)

add_custom_command(
    TARGET klik POST_BUILD
    COMMAND bash "-c" ./upload.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Upload binary to pi pico if connected"
    )

pico_add_extra_outputs(klik)