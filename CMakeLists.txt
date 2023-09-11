# pico-infonesPlus
cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)
# We also need PICO EXTRAS
include(pico_extras_import.cmake)

pico_sdk_init()

add_compile_options(-Wall)

add_subdirectory("../pico-vga-332/" "${CMAKE_CURRENT_BINARY_DIR}/pico-vga-332")

project(atari2600 C CXX ASM)

add_executable(atari2600
  atari/Atari-cart.c
  atari/Atari-memmap.c
  atari/Atari-TIA.c

  mos6507/mos6507-microcode.c
  mos6507/mos6507-opcodes.c

  mos6507/mos6507.c
  mos6532/mos6532.c

# test/debug.c

  main.c)

target_compile_definitions(atari2600  PRIVATE 
# VGA 222
VGA_ENC_RGB_222 
VGA_GPIO_FIRST=6 
VGA_GPIO_OUTNUM=6 
VGA_GPIO_SYNC=12
VGA_VSYNC=13
)

# Pull in our pico_stdlib which aggregates commonly used features
if(PICO_ON_DEVICE)
  target_link_libraries(
    atari2600
    pico_multicore
    pico_stdlib
    hardware_timer
    hardware_vreg
    hardware_dma
    pico_vga_332
    # tinyusb_board
    hardware_pio
    hardware_pwm)
else()  
# need SDL2
  find_package(SDL2 REQUIRED)

  target_include_directories(
    atari2600
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
              # have both forms of these because windows doesn't have SDL2
              # prefix
              ${SDL2_INCLUDE_DIR}
              ${SDL2_INCLUDE_DIR}/SDL2)

  target_link_libraries(atari2600 ${SDL2_LIBRARIES} pico_multicore pico_stdlib)
endif()

# enable for on pico debugging
pico_enable_stdio_usb(atari2600 0)
pico_enable_stdio_uart(atari2600 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(atari2600)

# add url via pico_set_program_url example_auto_set_url(atari2600)
