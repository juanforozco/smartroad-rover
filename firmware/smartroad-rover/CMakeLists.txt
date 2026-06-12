cmake_minimum_required(VERSION 3.13)

# Definir board como Pico W
set(PICO_BOARD pico_w)

# Importar el SDK del Pico
include(pico_sdk_import.cmake)

project(smartroad-rover C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializar el SDK
pico_sdk_init()

# Ejecutable principal con todos los modulos
add_executable(smartroad-rover
    smartroad-rover.c
    src/sensors.c
    src/motors.c
    src/navigation.c
    src/fsm.c
    src/web_server.c
)

# Incluir carpeta de headers
target_include_directories(smartroad-rover PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Librerias del SDK incluyendo WiFi
target_link_libraries(smartroad-rover
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    hardware_gpio
    hardware_pwm
    hardware_uart
    hardware_dma
    hardware_irq
    hardware_timer
)

# Salida por USB para debug
pico_enable_stdio_usb(smartroad-rover 1)
pico_enable_stdio_uart(smartroad-rover 0)

# Generar archivos UF2 y mapa
pico_add_extra_outputs(smartroad-rover)