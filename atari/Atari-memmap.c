/*
 * File: Atari-memmap.c
 * Author: dgrubb
 * Date: 06/28/2017
 *
 * Delegates access to memory mapped devices.
 */
// TODO remove
#include <stdio.h>


#include "Atari-memmap.h"
#include "Atari-cart.h"
#include "Atari-TIA.h"
#include "../mos6507/mos6507.h"
#include "../mos6532/mos6532.h"

#define IS_TIA(x)  (x >= MEMMAP_TIA_START && x <= MEMMAP_TIA_END)
#define IS_RIOT(x) ((x >= MEMMAP_RIOT_RAM_START && x <= MEMMAP_RIOT_RAM_END) || \
                    (x >= MEMMAP_RIOT_RAM_MIRROR_START && x <= MEMMAP_RIOT_RAM_MIRROR_END) || \
                    (x >= MEMMAP_RIOT_PERIPH_START && x <= MEMMAP_RIOT_PERIPH_END) || \
                    (x >= MEMMAP_RIOT_PERIPH_MIRROR_START && x <= MEMMAP_RIOT_PERIPH_MIRROR_END))
#define IS_CART(x) (x >= MEMMAP_CART_START && x <= MEMMAP_CART_END)

void memmap_map_address(uint16_t *address)
{
    /* The 6507 is a variant of the 6502. It shares the 16-bit addressing
     * system internally, but only connects to 13 external pins so the 
     * largest address which can be selected is 0x1FFF. However, as the 
     * processor can still handle full 16-bit addresses a programmer may have
     * used a greater value with the expectation those bits would be disregarded
     * and mirroring would take place.
     */
    *address = (*address & 0x1FFF);
}

/* These read/write functions essentially replicate the 
 * chip-select pins on the TIA and RIOT by delegating 
 * memory access requests to their respective locations.
 */
void memmap_write(void)
{
    /* Fetch data and address from CPU */
    uint16_t address;
    uint8_t data;
    mos6507_get_data_bus(&data);
    mos6507_get_address_bus(&address);
    memmap_map_address(&address);

    /* Access particular device */
    if (IS_TIA(address)) TIA_write_register(address - MEMMAP_TIA_START, data);
    if (IS_RIOT(address)) {
        memmap_map_riot_address(&address);
        mos6532_write(address, data);
    }
    if (IS_CART(address)) {
        /* Cartridges are read-only. Are there hardware peripherals which 
         * use this space for extending functionality? E.g., SuperCharger?
         */
    }
}

void memmap_read(uint8_t *data)
{
    /* Fetch address from CPU */
    uint16_t address;
    mos6507_get_address_bus(&address);
    memmap_map_address(&address);

    /* Access particular device */
    if (IS_TIA(address)) TIA_read_register(address - MEMMAP_TIA_START, data);
    if (IS_RIOT(address)) {
        memmap_map_riot_address(&address);
        mos6532_read(address, data);
    }
    if (IS_CART(address)) cartridge_read(address - MEMMAP_CART_START, data);

    mos6507_set_data_bus(*data);
}

void memmap_map_riot_address(uint16_t *address)
{
    if (*address >= MEMMAP_RIOT_RAM_START && *address <= MEMMAP_RIOT_RAM_END) {
        *address = *address - MEMMAP_RIOT_RAM_START;
    }
    if (*address >= MEMMAP_RIOT_RAM_MIRROR_START && *address <= MEMMAP_RIOT_RAM_MIRROR_END) {
        *address = *address - MEMMAP_RIOT_RAM_MIRROR_START;
    }
    if (*address >= MEMMAP_RIOT_PERIPH_MIRROR_START && *address <= MEMMAP_RIOT_PERIPH_MIRROR_END) {
        *address = *address - 0x100;
    }
}
