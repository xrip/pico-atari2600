/*
 * File: debug.h
 * Author: dgrubb
 * Date: 08/07/2017
 *
 * Provides helper functions for printing human-readable 
 * information.
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef PRINT_STATE

#include "../mos6507/mos6507.h"

typedef enum {
    DEBUG_STACK_ACTION_PUSH = 0,
    DEBUG_STACK_ACTION_PULL,
    DEBUG_STACK_ACTION_NONE
} debug_stack_action_t;

typedef struct {
    uint8_t op;
    const char *str;
} debug_opcode_t;

const char * debug_lookup_opcode_str(uint8_t opcode);
void debug_print_status_flags(void);
void debug_print_memory_contents(uint16_t address);
void debug_print_special_register(mos6507_register_t reg);
void debug_print_buses(void);
void debug_print_execution_step(void);
int debug_get_status_flag(uint8_t flag);
void debug_print_instruction(void);
void debug_print_illegal_opcode(uint8_t opcode);
const char * debug_lookup_opcode_str(uint8_t opcode);
void debug_print_stack_action(debug_stack_action_t action);
void debug_print_stack();
void debug_print_timer();

#endif /* PRINT_STATE */

#endif /* _DEBUG_H */
