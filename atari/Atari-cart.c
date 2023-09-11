/*
 * File: Atari-cart.c
 * Author: dgrubb
 * Date: 07/07/2017
 *
 * Mimics ROM space (that is, a game cartridge).
 */

#include "Atari-cart.h"

/* Cartridges are represented as arrays of bytes in their own
 * part of memory. We "load" a cartridge by storing a pointer 
 * to the desired cartridge data.
 */
static const uint8_t *cartridge = 0;

void cartridge_read(uint16_t address, uint8_t * data)
{
    if (cartridge) {
        *data = cartridge[address];
    }
}

void cartridge_load(const uint8_t *cart)
{
    if (cartridge) {
        cartridge_eject();
    }
    cartridge = cart;
}

void cartridge_eject(void)
{
    /* Clear the pointer to the current cartridge array */
    cartridge = 0;
}

