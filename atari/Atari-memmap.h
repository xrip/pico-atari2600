/*
 * File: Atari-memmap.h
 * Author: dgrubb
 * Date: 06/28/2017
 *
 * Delegates access to memory mapped devices.
 */

#ifndef _MEMMAP_H
#define _MEMMAP_H

#include <stdint.h>

#define MEMMAP_TIA_START                0x0000
#define MEMMAP_TIA_END                  0x007F
#define MEMMAP_RIOT_RAM_START           0x0080
#define MEMMAP_RIOT_RAM_END             0x00FF
#define MEMMAP_RIOT_RAM_MIRROR_START    0x0180
#define MEMMAP_RIOT_RAM_MIRROR_END      0x01FF
#define MEMMAP_RIOT_PERIPH_START        0x0280
#define MEMMAP_RIOT_PERIPH_END          0x0297
#define MEMMAP_RIOT_PERIPH_MIRROR_START 0x0380
#define MEMMAP_RIOT_PERIPH_MIRROR_END   0x0397
#define MEMMAP_CART_START               0x1000
#define MEMMAP_CART_END                 0x1FFF

void memmap_write(void);
void memmap_read(uint8_t *data);
void memmap_map_address(uint16_t *address);
void memmap_map_riot_address(uint16_t *address);

#endif /* _MEMMAP_H */
