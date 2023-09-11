/*
 * File: mos6532.h
 * Author: dgrubb
 * Date: 06/26/2017
 *
 * Provides implementation of MOS 6532 RAM-I/O-Timer (RIOT) chip. This handles
 * both 128 bytes of RAM storage and digital I/O.
 */

#ifndef _MOS6532_H
#define _MOS6532_H

#include <stdint.h>
#include <string.h>

#define MEM_SIZE 128

typedef enum {
    SWCHA = 0x280,
    SWACNT,
    SWCHB,
    SWBCNT,
} mos6532_readable_register_t;

#define MOS6532_MEMMAP_INTIM    0x284
#define MOS6532_MEMMAP_TIM1T    0x294
#define MOS6532_MEMMAP_TIM8T    0x295
#define MOS6532_MEMMAP_TIM64T   0x296
#define MOS6532_MEMMAP_TIM1024T 0x297



typedef enum {
    MOS6532_TIMER_DIVISOR_NONE = 0,
    MOS6532_TIMER_DIVISOR_T1 = 1,
    MOS6532_TIMER_DIVISOR_T8 = 8,
    MOS6532_TIMER_DIVISOR_T64 = 64,
    MOS6532_TIMER_DIVISOR_T1024 = 1024
} mos6532_timer_divisor_t;

typedef struct {
    uint8_t counter;
    uint8_t interval_timer;
    uint8_t fired;
    mos6532_timer_divisor_t timer_set;
} mos6532_timer_t;

//mos6532_timer_t timer;

/* Utility functions */
int mos6532_bounds_check(uint16_t address);
void mos6532_clear_memory(void);
void mos6532_init(void);
int mos6532_set_timer(mos6532_timer_divisor_t divisor, uint8_t data);
void mos6532_timer_interval(mos6532_timer_divisor_t divisor);
/* External memory access */
int mos6532_read(uint16_t address, uint8_t *data);
int mos6532_write(uint16_t address, uint8_t data);
void mos6532_clock_tick(void);
void mos6532_get_interval(mos6532_timer_divisor_t *divisor);
void mos6532_get_counter(uint8_t *counter);
char * mos6532_get_divisor_str(mos6532_timer_divisor_t divisor);
void mos6532_map_mirrored_addresses(uint16_t *address);

#endif /* _MOS6532_H */

