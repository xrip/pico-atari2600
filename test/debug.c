/*
 * File: debug.c
 * Author: dgrubb
 * Date: 08/07/2017
 *
 * Provides helper functions for printing human-readable
 * information.
 */

//#ifdef PRINT_STATE

#define PRINT_STATE 
#include <stdio.h>
//#include <string.h>
//#include <unistd.h>
#include "debug.h"
#include "../atari/Atari-memmap.h"
#include "../mos6532/mos6532.h"

#define MOS6507_STATUS_FLAG_NEGATIVE  0x80
#define MOS6507_STATUS_FLAG_OVERFLOW  0x40
#define MOS6507_STATUS_FLAG_BREAK     0x10
#define MOS6507_STATUS_FLAG_DECIMAL   0x08
#define MOS6507_STATUS_FLAG_INTERRUPT 0x04
#define MOS6507_STATUS_FLAG_ZERO      0x02
#define MOS6507_STATUS_FLAG_CARRY     0x01

#define MSG_LEN 300

#define IS_TIA(x)  (x >= MEMMAP_TIA_START && x <= MEMMAP_TIA_END)
#define IS_RIOT(x) (x >= MEMMAP_RIOT_RAM_START && x <= MEMMAP_RIOT_PERIPH_MIRROR_END)
#define IS_CART(x) (x >= MEMMAP_CART_START && x <= MEMMAP_CART_END)

debug_opcode_t debug_opcodes[] = {
    { 0x00, "None"},
    /* LDA */
    {0xA9, "LDA - Immediate"},
    {0xA5, "LDA - Zero page"},
    {0xB5, "LDA - Zero page X indexed"},
    {0xAD, "LDA - Absolute"},
    {0xBD, "LDA - Absolute X indexed"},
    {0xB9, "LDA - Absolute Y indexed"},
    {0xA1, "LDA - Indirect X indexed"},
    {0xB1, "LDA - Indirect Y indexed"},
    /* LDX */
    {0xA2, "LDX - Immediate"},
    {0xA6, "LDX - Zero page"},
    {0xB6, "LDX - Zero page Y indexed"},
    {0xAE, "LDX - Absolute"},
    {0xBE, "LDX - Absolute Y indexed"},
    /* LDY */
    {0xA0, "LDY - Immediate"},
    {0xA4, "LDY - Zero page"},
    {0xB4, "LDY - Zero page X indexed"},
    {0xAC, "LDY - Absolute"},
    {0xBC, "LDY - Absolute X indexed"},
    /* STA */
    {0x85, "STA - Zero page"},
    {0x95, "STA - Zero page X indexed"},
    {0x8D, "STA - Absolute"},
    {0x9D, "STA - Absolute X indexed"},
    {0x99, "STA - Absolute Y indexed"},
    {0x81, "STA - Indirect X indexed"},
    {0x91, "STA - Indirect Y indexed"},
    /* STX */
    {0x86, "STX - Zero page"},
    {0x96, "STX - Zero page Y indexed"},
    {0x8E, "STX - Absolute"},
    /* STY */
    {0x84, "STY - Zero page"},
    {0x94, "STY - Zero page Y indexed"},
    {0x8E, "STY - Absolute"},
    /* ADC */
    {0x69, "ADC - Immediate"},
    {0x65, "ADC - Zero page"},
    {0x75, "ADC - Zero page X indexed"},
    {0x6D, "ADC - Absolute"},
    {0x7D, "ADC - Absolute X indexed"},
    {0x79, "ADC - Absolute Y indexed"},
    {0x61, "ADC - Indirect X indexed"},
    {0x71, "ADC - Indirect Y indexed"},
    /* SBC */
    {0xE9, "SBC - Immediate"},
    {0xE5, "SBC - Zero page"},
    {0xF5, "SBC - Zero page X indexed"},
    {0xED, "SBC - Absolute"},
    {0xFD, "SBC - Absolute X indexed"},
    {0xF9, "SBC - Absolute Y indexed"},
    {0xE1, "SBC - Indirect X indexed"},
    {0xF1, "SBC - Indirect Y indexed"},
    /* INC */
    {0xE6, "INC - Zero page"},
    {0xF6, "INC - Zero page X indexed"},
    {0xEE, "INC - Absolute"},
    {0xFE, "INC - Absolute X indexed"},
    /* INX */
    {0xE8, "INX - Implied"},
    /* INY */
    {0xFE, "INY - Implied"},
    /* DEC */
    {0xC6, "DEC - Zero page"},
    {0xD6, "DEC - Zero page X indexed"},
    {0xCE, "DEC - Absolute"},
    {0xDE, "DEC - Absolute X indexed"},
    /* DEX */
    {0xCA, "DEX - Implied"},
    /* DEY */
    {0x88, "DEY - Implied"},
    /* AND */
    {0x21, "AND - Indirect X indexed"},
    {0x25, "AND - Zero page"},
    {0x29, "AND - Immediate"},
    {0x2D, "AND - Absolute"},
    {0x31, "AND - Indirect Y indexed"},
    {0x35, "AND - Zero page X indexed"},
    {0x39, "AND - Absolute Y indexed"},
    {0x3D, "AND - Absolute X indexed"},
    /* ORA */
    {0x01, "ORA - Indirect X indexed"},
    {0x05, "ORA - Zero page"},
    {0x09, "ORA - Immediate"},
    {0x0D, "ORA - Absolute"},
    {0x11, "ORA - Indirect Y indexed"},
    {0x15, "ORA - Zero page X indexed"},
    {0x19, "ORA - Absolute Y indexed"},
    {0x1D, "ORA - Absolute X indexed"},
    /* EOR */
    {0x41, "EOR - Indirect X indexed"},
    {0x45, "EOR - Zero page"},
    {0x49, "EOR - Immediate"},
    {0x4D, "EOR - Absolute"},
    {0x51, "EOR - Indirect Y indexed"},
    {0x55, "EOR - Zero page X indexed"},
    {0x59, "EOR - Absolute Y indexed"},
    {0x5D, "EOR - Absolute X indexed"},
    /* JMP */
    {0x4C, "JMP - Absolute"},
    {0x6C, "JMP - Indirect"},
    /* BCC */
    {0x90, "BCC - Relative"},
    /* BCS */
    {0xB0, "BCS - Relative"},
    /* BEQ */
    {0xF0, "BEQ - Relative"},
    /* BNE */
    {0xD0, "BNE - Relative"},
    /* BMI */
    {0x30, "BMI - Relative"},
    /* BPL */
    {0x10, "BPL - Relative"},
    /* BVS */
    {0x70, "BVS - Relative"},
    /* BVC */
    {0x50, "BVC - Relative"},
    /* CMP */
    {0xC9, "CMP - Immediate"},
    {0xC5, "CMP - Zero page"},
    {0xD5, "CMP - Zero page X indexed"},
    {0xCD, "CMP - Absolute"},
    {0xDD, "CMP - Absolute X indexed"},
    {0xD9, "CMP - Absolute Y indexed"},
    {0xC1, "CMP - Indirect X indexed"},
    {0xD1, "CMP - Indirect Y indexed"},
    /* CPX */
    {0xE0, "CPX - Immediate"},
    {0xE4, "CPX - Zero page"},
    {0xEC, "CPX - Absolute"},
    /* CPY */
    {0xC0, "CPY - Immediate"},
    {0xC4, "CPY - Zero page"},
    {0xCC, "CPY - Absolute"},
    /* BIT */
    {0x24, "BIT - Zero page"},
    {0x2C, "BIT - Absolute"},
    /* ASL */
    {0x0A, "ASL - Accumulator"},
    {0x06, "ASL - Zero page"},
    {0x16, "ASL - Zero page X indexed"},
    {0x0E, "ASL - Absolute"},
    {0x1E, "ASL - Absolute X indexed"},
    /* LSR */
    {0x4A, "LSR - Accumulator"},
    {0x46, "LSR - Zero page"},
    {0x56, "LSR - Zero page X indexed"},
    {0x4E, "LSR - Absolute"},
    {0x5E, "LSR - Absolute X indexed"},
    /* ROL */
    {0x2A, "ROL - Accumulator"},
    {0x26, "ROL - Zero page"},
    {0x36, "ROL - Zero page X indexed"},
    {0x2E, "ROL - Absolute"},
    {0x3E, "ROL - Absolute X indexed"},
    /* ROR */
    {0x6A, "ROR - Accumulator"},
    {0x66, "ROR - Zero page"},
    {0x76, "ROR - Zero page X indexed"},
    {0x6E, "ROR - Absolute"},
    {0x7E, "ROR - Absolute X indexed"},
    /* TAX */
    {0xAA, "TAX - Implied"},
    /* TAY */
    {0xA8, "TAY - Implied"},
    /* TXA */
    {0x8A, "TXA - Implied"},
    /* TYA */
    {0x98, "TYA - Implied"},
    /* TSX */
    {0xBA, "TSX - Implied"},
    /* TXS */
    {0x9A, "TXS - Implied"},
    /* PHA */
    {0x48, "PHA - Implied"},
    /* PHP */
    {0x08, "PHP - Implied"},
    /* PLA */
    {0x68, "PLA - Implied"},
    /* PLP */
    {0x28, "PLP - Implied"},
    /* JSR */
    {0x20, "JSR - Implied"},
    /* RTS */
    {0x60, "RTS - Implied"},
    /* RTI */
    {0x40, "RTI - Implied"},
    /* CLC */
    {0x18, "CLC - Implied"},
    /* CLD */
    {0xD8, "CLD - Implied"},
    /* CLI */
    {0x58, "CLI - Implied"},
    /* CLV */
    {0xB8, "CLV - Implied"},
    /* SEC */
    {0x38, "SEC - Implied"},
    /* SED */
    {0xF8, "SED - Implied"},
    /* SEI */
    {0x78, "SEI - Implied"},
    /* NOP */
    {0xEA, "NOP - Implied"}
};

const uint8_t debug_opcode_table_size = sizeof debug_opcodes / sizeof debug_opcodes[0];

#ifdef PRINT_STATE
int debug_get_status_flag(uint8_t flag)
{
    uint8_t p;
    mos6507_get_register(MOS6507_REG_P, &p);
    switch (flag) {
        case MOS6507_STATUS_FLAG_NEGATIVE:
            return (p & MOS6507_STATUS_FLAG_NEGATIVE);
        case MOS6507_STATUS_FLAG_OVERFLOW:
            return (p & MOS6507_STATUS_FLAG_OVERFLOW);
        case MOS6507_STATUS_FLAG_BREAK:
            return (p & MOS6507_STATUS_FLAG_BREAK);
        case MOS6507_STATUS_FLAG_DECIMAL:
            return (p & MOS6507_STATUS_FLAG_DECIMAL);
        case MOS6507_STATUS_FLAG_INTERRUPT:
            return (p & MOS6507_STATUS_FLAG_INTERRUPT);
        case MOS6507_STATUS_FLAG_ZERO:
            return (p & MOS6507_STATUS_FLAG_ZERO);
        case MOS6507_STATUS_FLAG_CARRY:
            return (p & MOS6507_STATUS_FLAG_CARRY);
        default:
            return 0;
    }
}

void debug_print_memory_contents(uint16_t address)
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);

    char *template = "Memory contents at address [ 0x%X, %d ]: 0x%X, %d\n\r";

    uint16_t current_address;
    uint8_t data;
    // Change the address bus to the specified location, read the data, then
    // revert back to what it was without a full CPU clock tick to make the 
    // read seamless
    mos6507_get_address_bus(&current_address);
    mos6507_set_address_bus(address);
    memmap_read(&data);
    mos6507_set_address_bus(current_address);
    printf(template, address, address, data, data);
//    puts(msg);
}

void debug_print_special_register(mos6507_register_t reg)
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);

    char *template = "Special register [ %s ] contents: 0x%X, %d\n\r";

    uint8_t data;
    uint16_t longdata;

    if(reg == MOS6507_REG_PC) {
       longdata = mos6507_get_PC();
       printf(template, mos6507_get_register_str(reg), longdata, longdata);
    } else {
        mos6507_get_register(reg, &data);
        printf(template, mos6507_get_register_str(reg), data, data);
    }
//    puts(msg);
}

void debug_print_status_flags(void)
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);

    char *template = "Processor flags:\n\r"
                     "|- Sign -|- Overflow -|- Break -|- Decimal -|- Interrupt -|- Zero -|- Carry -|\n\r"
                     "|    %d   |     %d      |    %d    |     %d     |      %d      |   %d    |    %d    |\n\r";

    printf(template,
            mos6507_get_status_flag(MOS6507_STATUS_FLAG_NEGATIVE),
            mos6507_get_status_flag(MOS6507_STATUS_FLAG_OVERFLOW),
            mos6507_get_status_flag(MOS6507_STATUS_FLAG_BREAK),
            mos6507_get_status_flag(MOS6507_STATUS_FLAG_DECIMAL),
            mos6507_get_status_flag(MOS6507_STATUS_FLAG_INTERRUPT),
            mos6507_get_status_flag(MOS6507_STATUS_FLAG_ZERO),
            mos6507_get_status_flag(MOS6507_STATUS_FLAG_CARRY));

//    puts(msg);
}

void debug_print_buses(void)
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);
    uint16_t address;
    uint16_t offset_address;
    uint8_t data;
    char *subsystem;
    mos6507_get_data_bus(&data);
    mos6507_get_address_bus(&address);

    memmap_map_address(&address);

    if (IS_TIA(address)) {
        subsystem = "TIA";
        offset_address = address - MEMMAP_TIA_START;
    }
    if (IS_RIOT(address)) {
        subsystem = "RIOT";
        offset_address = address - MEMMAP_RIOT_RAM_START;
    }
    if (IS_CART(address)) {
        subsystem = "CART";
        offset_address = address - MEMMAP_CART_START;
    }

    char *template = "Address bus [ 0x%X, %s access: 0x%X ], data bus [ 0x%X ]\n\r";

    printf(template, address, subsystem, offset_address, data);
//    puts(msg);
}

void debug_print_instruction(void)
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);

    uint8_t instruction, cycle;

    mos6507_get_current_instruction(&instruction);
    mos6507_get_current_instruction_cycle(&cycle);

    char * template = "Instruction [ 0x%X, %s ], cycle: %d\n\r";

    printf(template, instruction, debug_lookup_opcode_str(instruction), cycle);
//    puts(msg);
}

void debug_print_illegal_opcode(uint8_t opcode)
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);

    char * template = "\n\r!!! Error: Illegal opcode [ 0x%X ] !!!\n\r";

    printf(template, opcode);
//    puts(msg);
    debug_print_execution_step();
}

void debug_print_stack_action(debug_stack_action_t action)
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);
    uint16_t address;
    uint8_t data;
    char * template = "%s stack ..\n\r%s\tStack address: 0x%X, data: 0x%X\n\r";

    mos6507_get_address_bus(&address);
    mos6507_get_data_bus(&data);

    puts("----------------------\n\r");
    if (action == DEBUG_STACK_ACTION_PUSH) {
        printf(template, "Pushing to", "-->", address, data);
    }
    if (action == DEBUG_STACK_ACTION_PULL) {
        printf(template, "Pulling from", "<--", address, data);
    }
//    puts(msg);
}

void debug_print_stack()
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);

    char * template = "[ 0x%X ]\tData: 0x%X\n\r";

    uint16_t address_bus;
    uint8_t i, sp, data_bus, data;
    mos6507_get_address_bus(&address_bus);
    mos6507_get_data_bus(&data_bus);
    mos6507_get_register(MOS6507_REG_S, &sp);

    puts("----------------------\n\r");

    printf("Stack pointer: 0x%X\n\r", sp);
//    puts(msg);
    memset(msg, 0, MSG_LEN);

    if (sp == 0xFF) {
        puts("Stack empty\n\r");
    } else {
        for (i=0xFF; i>sp; i--) {
            mos6507_set_address_bus_hl(STACK_PAGE, i);
            memmap_read(&data);
            printf(template, i, data);
        //    puts(msg);
        }
    }

    mos6507_set_address_bus(address_bus);
    mos6507_set_data_bus(data_bus);
}

void debug_print_timer()
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);

    uint8_t counter, interrupt;
    mos6532_timer_divisor_t divisor;

    char * template = "Timer [ %s SET: %d ], interval: %s\n\r";

    mos6532_get_counter(&counter);
    mos6532_get_interval(&divisor);

    printf(template,
            (counter ? "" : "NOT"),
            counter,
            mos6532_get_divisor_str(divisor));
//    puts(msg);
}

void debug_print_execution_step(void)
{
    printf("\n\r----------------------------------------------------------------"
         "---------------------\n\r");

    debug_print_instruction();
    debug_print_buses();
    debug_print_special_register(MOS6507_REG_PC);
    debug_print_special_register(MOS6507_REG_A);
    debug_print_special_register(MOS6507_REG_S);
    debug_print_special_register(MOS6507_REG_X);
    debug_print_special_register(MOS6507_REG_Y);
    debug_print_timer();
    debug_print_status_flags();
    debug_print_stack();
}

const char * debug_lookup_opcode_str(uint8_t opcode)
{
    int i;
    for (i=0; i<debug_opcode_table_size; i++) {
        if (debug_opcodes[i].op == opcode) {
            return debug_opcodes[i].str;
        }
    }
    return "Unknown";
}

#endif /* PRINT_STATE */
