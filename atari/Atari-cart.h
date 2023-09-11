/*
 * File: Atari-cart.h
 * Author: dgrubb
 * Date: 07/07/2017
 *
 * Mimics ROM space (that is, a game cartridge).
 */

#ifndef _ATARI_CART_H
#define _ATARI_CART_H

#include <stdint.h>

void cartridge_read(uint16_t address, uint8_t * data);
void cartridge_load(const uint8_t *cart);
void cartridge_eject(void);

#endif /* _ATARI_CART_H */
