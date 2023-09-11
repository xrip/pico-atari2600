/*
 * File: mos6532.c
 * Author: dgrubb
 * Date: 06/26/2017
 *
 * Provides implementation of MOS 6532 RAM-I/O-Timer (RIOT) chip. This handles
 * both 128 bytes of RAM storage and digital I/O.
 */

#include <string.h>
#include "../mos6507/mos6507.h"
#include "mos6532.h"

static uint8_t memory[MEM_SIZE];
mos6532_timer_t timer;
uint8_t joy1_state = 0xFF;
uint8_t switches_state = 0b00001011;

void mos6532_init(void)
{
    timer = (mos6532_timer_t){0};
    timer.timer_set = MOS6532_TIMER_DIVISOR_NONE;
    timer.interval_timer = 0;
    timer.counter = 0;
    mos6532_clear_memory();
}

/* Checks that a requested memory address is within the 
 * RAM space.
 *
 * address: address to verify.
 *
 * Returns 0 on success, -1 on error.
 */
int mos6532_bounds_check(uint16_t address)
{
    if (address > MEM_SIZE) {
        return -1;
    }
    /* Flag an error */
    return 0;
}

/* Resets all RAM to zero
 */
void mos6532_clear_memory(void)
{
    memset(memory, 0, MEM_SIZE);
}

/* Loads a value from within RAM and places it into 
 * a variable given by pointer.
 *
 * Returns 0 on success, -1 on error.
 */
int mos6532_read(uint16_t address, uint8_t *data)
{
    mos6532_map_mirrored_addresses(&address);
    /* Check whether special peripheral registers are the target of the read
     * before passing over to the general RAM area.
     */
    switch (address) {
        case SWCHA:
            *data = joy1_state;
            return 0;
        case SWACNT:
            *data = 0x00;
            return 0;
        case SWCHB:
            *data = switches_state;
            return 0;
        case SWBCNT:
            *data = 0x00;
            return 0;
        case MOS6532_MEMMAP_INTIM:
            *data = timer.counter;
            return 0;
    }
    if (-1 == mos6532_bounds_check(address)) {
        /* Error, attempting to read outside memory */
        return -1;
    }
    *data = memory[address];
    return 0;
}

int mos6532_set_timer(mos6532_timer_divisor_t divisor, uint8_t data)
{
    timer.timer_set = divisor;
    timer.interval_timer = divisor;
    timer.counter = data;
    timer.fired = 0;
}

/* Writes to a RAM address.
 *
 * Returns 0 on success, -1 on error.
 */
int mos6532_write(uint16_t address, uint8_t data)
{
    mos6532_map_mirrored_addresses(&address);
    /* Check whether special peripheral registers are the target of the write
     * before passing over to the general RAM area.
     */
    switch (address) {
        case SWCHA:
        joy1_state = data;
            return 0;
        case SWCHB:
        switches_state = data;
        return 0;
        case SWACNT:
        case SWBCNT:
            return 0;
        case MOS6532_MEMMAP_TIM1T:
            mos6532_set_timer(MOS6532_TIMER_DIVISOR_T1, data);
            return 0;
        case MOS6532_MEMMAP_TIM8T:
            mos6532_set_timer(MOS6532_TIMER_DIVISOR_T8, data);
            return 0;
        case MOS6532_MEMMAP_TIM64T:
            mos6532_set_timer(MOS6532_TIMER_DIVISOR_T64, data);
            return 0;
        case MOS6532_MEMMAP_TIM1024T:
            mos6532_set_timer(MOS6532_TIMER_DIVISOR_T1024, data);
            return 0;
    }
    if (-1 == mos6532_bounds_check(address)) {
        /* Error, attempting to write outside memory */
        return -1;
    }
    memory[address] = data;
    return 0;
}

void mos6532_timer_interval(mos6532_timer_divisor_t divisor)
{
    timer.interval_timer--;
    if (timer.interval_timer == 0) {
        timer.interval_timer = timer.timer_set;
        timer.counter--;
        if (timer.fired == 1) {
            timer.timer_set = MOS6532_TIMER_DIVISOR_NONE;
        }
    }
    if (timer.counter == 0) {
        timer.fired = 1;
    }
}

void mos6532_clock_tick(void)
{
    switch (timer.timer_set) {
        case MOS6532_TIMER_DIVISOR_T1:
            mos6532_timer_interval(MOS6532_TIMER_DIVISOR_T1);
            break;
        case MOS6532_TIMER_DIVISOR_T8:
            mos6532_timer_interval(MOS6532_TIMER_DIVISOR_T8);
            break;
        case MOS6532_TIMER_DIVISOR_T64:
            mos6532_timer_interval(MOS6532_TIMER_DIVISOR_T64);
            break;
        case MOS6532_TIMER_DIVISOR_T1024:
            mos6532_timer_interval(MOS6532_TIMER_DIVISOR_T1024);
            break;
        case MOS6532_TIMER_DIVISOR_NONE:
            timer.counter--;
            break;
        default:
            /* No timer set */
            break;
    }
}


void mos6532_get_interval(mos6532_timer_divisor_t *divisor)
{
    *divisor = timer.timer_set;
}

void mos6532_get_counter(uint8_t *counter)
{
    *counter = timer.counter;
}

char * mos6532_get_divisor_str(mos6532_timer_divisor_t divisor)
{
    switch (divisor) {
        case MOS6532_TIMER_DIVISOR_T1:    return "TIM1T - 1";
        case MOS6532_TIMER_DIVISOR_T8:    return "TIM8T - 8";
        case MOS6532_TIMER_DIVISOR_T64:   return "TIM64T - 64";
        case MOS6532_TIMER_DIVISOR_T1024: return "TIM1024T - 1024";
        default:             return "Unknown";
    }
}

void mos6532_map_mirrored_addresses(uint16_t *address)
{
    if (*address & 0x0100) {
        *address &= 0x00FF;
    }
}

