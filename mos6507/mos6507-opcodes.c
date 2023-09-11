/*
 * File: mos6507-opcodes.c
 * Author: dgrubb
 * Date: 12/20/2016
 *
 * Provides implementations of the 6507's op-codes.
 *
 * Useful information about cycle execution times:
 * http://users.telenet.be/kim1-6502/6502/hwman.html##AA
 *
 * Useful information about opcode logic and status flags:
 * http://www.obelisk.me.uk/6502/reference.html
 */

#include "../atari/Atari-memmap.h"
#include "mos6507.h"
#include "mos6507-opcodes.h"
#include "mos6507-microcode.h"
#include "mos6507-addressing-macros.h"

/* Invoked at the end of each op-code. Resets the
 * clock cycle counter and increments the PC and 
 * address busses for the next op-code
 */
#define END_OPCODE() \
    mos6507_increment_PC(); \
    mos6507_set_address_bus(mos6507_get_PC());

#define STACK_PAGE 0x01

instruction_t ISA_table[ISA_LENGTH];

/* Looks up an instruction from the instruction table and
 * executes the corresponding function, passing along cycle
 * time and addressing mode.
 *
 * opcode: value of the instruction, e.g., 0x00 for BRK
 */
int opcode_execute(uint8_t opcode)
{
    static int cycle = 0;
    if (-1 == ISA_table[opcode].opcode(cycle, ISA_table[opcode].addressing_mode)) {
        cycle++;
    } else {
        cycle = 0;
    }
    return cycle;
}

int opcode_validate(uint8_t opcode)
{
    if (ISA_table[opcode].opcode == opcode_ILL) {
        return -1;
    }
    return 0;
}

/* Loads an array with function pointers to the corresponding
 * instruction. E.g., LDY is 0x0C so index 12 of the ISA table
 * would be loaded with a pointer to function opcode_LDY().
 */
void opcode_populate_ISA_table(void)
{
    int i = 0;
    /* Initially, fill the entire table with ILL */
    for (i=0; i<ISA_LENGTH; i++) {
        ISA_table[i].opcode = opcode_ILL;
        ISA_table[i].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;
    }

    /* Now fill out the actual codes supported by a real 6507 */

    /* 0x00: BRK, Implied */
    ISA_table[0x00].opcode = opcode_BRK;
    ISA_table[0x00].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Load accumulator with memory */
    ISA_table[0xA9].opcode = opcode_LDA;
    ISA_table[0xA9].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0xA5].opcode = opcode_LDA;
    ISA_table[0xA5].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0xB5].opcode = opcode_LDA;
    ISA_table[0xB5].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0xAD].opcode = opcode_LDA;
    ISA_table[0xAD].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0xBD].opcode = opcode_LDA;
    ISA_table[0xBD].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;
    ISA_table[0xB9].opcode = opcode_LDA;
    ISA_table[0xB9].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_Y_INDEXED;
    ISA_table[0xA1].opcode = opcode_LDA;
    ISA_table[0xA1].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_X_INDEXED;
    ISA_table[0xB1].opcode = opcode_LDA;
    ISA_table[0xB1].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_Y_INDEXED;

    /* Load Index X with memory */
    ISA_table[0xA2].opcode = opcode_LDX;
    ISA_table[0xA2].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0xA6].opcode = opcode_LDX;
    ISA_table[0xA6].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0xB6].opcode = opcode_LDX;
    ISA_table[0xB6].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_Y_INDEXED;
    ISA_table[0xAE].opcode = opcode_LDX;
    ISA_table[0xAE].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0xBE].opcode = opcode_LDX;
    ISA_table[0xBE].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_Y_INDEXED;

    /* Load Index Y with memory */
    ISA_table[0xA0].opcode = opcode_LDY;
    ISA_table[0xA0].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0xA4].opcode = opcode_LDY;
    ISA_table[0xA4].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0xB4].opcode = opcode_LDY;
    ISA_table[0xB4].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0xAC].opcode = opcode_LDY;
    ISA_table[0xAC].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0xBC].opcode = opcode_LDY;
    ISA_table[0xBC].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;

    /* Store Accumulator in memory */
    ISA_table[0x85].opcode = opcode_STA;
    ISA_table[0x85].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x95].opcode = opcode_STA;
    ISA_table[0x95].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0x8D].opcode = opcode_STA;
    ISA_table[0x8D].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0x9D].opcode = opcode_STA;
    ISA_table[0x9D].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;
    ISA_table[0x99].opcode = opcode_STA;
    ISA_table[0x99].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_Y_INDEXED;
    ISA_table[0x81].opcode = opcode_STA;
    ISA_table[0x81].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_X_INDEXED;
    ISA_table[0x91].opcode = opcode_STA;
    ISA_table[0x91].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_Y_INDEXED;

    /* Store Index X in memory */
    ISA_table[0x86].opcode = opcode_STX;
    ISA_table[0x86].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x96].opcode = opcode_STX;
    ISA_table[0x96].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_Y_INDEXED;
    ISA_table[0x8E].opcode = opcode_STX;
    ISA_table[0x8E].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;

    /* Store Index Y in memory */
    ISA_table[0x84].opcode = opcode_STY;
    ISA_table[0x84].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x94].opcode = opcode_STY;
    ISA_table[0x94].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0x8C].opcode = opcode_STY;
    ISA_table[0x8C].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;

    /* Add memory to Accumulator with carry */
    ISA_table[0x69].opcode = opcode_ADC;
    ISA_table[0x69].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0x65].opcode = opcode_ADC;
    ISA_table[0x65].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x75].opcode = opcode_ADC;
    ISA_table[0x75].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0x6D].opcode = opcode_ADC;
    ISA_table[0x6D].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0x7D].opcode = opcode_ADC;
    ISA_table[0x7D].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;
    ISA_table[0x79].opcode = opcode_ADC;
    ISA_table[0x79].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_Y_INDEXED;
    ISA_table[0x61].opcode = opcode_ADC;
    ISA_table[0x61].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_X_INDEXED;
    ISA_table[0x71].opcode = opcode_ADC;
    ISA_table[0x71].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_Y_INDEXED;

    /* Subtract memory from Accumulator with borrow */
    ISA_table[0xE9].opcode = opcode_SBC;
    ISA_table[0xE9].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0xE5].opcode = opcode_SBC;
    ISA_table[0xE5].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0xF5].opcode = opcode_SBC;
    ISA_table[0xF5].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0xED].opcode = opcode_SBC;
    ISA_table[0xED].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0xFD].opcode = opcode_SBC;
    ISA_table[0xFD].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;
    ISA_table[0xF9].opcode = opcode_SBC;
    ISA_table[0xF9].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_Y_INDEXED;
    ISA_table[0xE1].opcode = opcode_SBC;
    ISA_table[0xE1].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_X_INDEXED;
    ISA_table[0xF1].opcode = opcode_SBC;
    ISA_table[0xF1].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_Y_INDEXED;

    /* Increment memory by one */
    ISA_table[0xE6].opcode = opcode_INC;
    ISA_table[0xE6].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0xF6].opcode = opcode_INC;
    ISA_table[0xF6].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0xEE].opcode = opcode_INC;
    ISA_table[0xEE].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0xFE].opcode = opcode_INC;
    ISA_table[0xFE].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;

    /* Increment Index X by one */
    ISA_table[0xE8].opcode = opcode_INX;
    ISA_table[0xE8].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Increment Index Y by one */
    ISA_table[0xC8].opcode = opcode_INY;
    ISA_table[0xC8].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Decrement memory by one */
    ISA_table[0xC6].opcode = opcode_DEC;
    ISA_table[0xC6].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0xD6].opcode = opcode_DEC;
    ISA_table[0xD6].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0xCE].opcode = opcode_DEC;
    ISA_table[0xCE].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0xDE].opcode = opcode_DEC;
    ISA_table[0xDE].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;

    /* Decrement Index X by one */
    ISA_table[0xCA].opcode = opcode_DEX;
    ISA_table[0xCA].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Decrement Index Y by one */
    ISA_table[0x88].opcode = opcode_DEY;
    ISA_table[0x88].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Logical AND with Accumulator */
    ISA_table[0x21].opcode = opcode_AND;
    ISA_table[0x21].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_X_INDEXED;
    ISA_table[0x25].opcode = opcode_AND;
    ISA_table[0x25].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x29].opcode = opcode_AND;
    ISA_table[0x29].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0x2D].opcode = opcode_AND;
    ISA_table[0x2D].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0x31].opcode = opcode_AND;
    ISA_table[0x31].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_Y_INDEXED;
    ISA_table[0x35].opcode = opcode_AND;
    ISA_table[0x35].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0x39].opcode = opcode_AND;
    ISA_table[0x39].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_Y_INDEXED;
    ISA_table[0x3D].opcode = opcode_AND;
    ISA_table[0x3D].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;

    /* Logical OR with Accumulator */
    ISA_table[0x01].opcode = opcode_ORA;
    ISA_table[0x01].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_X_INDEXED;
    ISA_table[0x05].opcode = opcode_ORA;
    ISA_table[0x05].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x09].opcode = opcode_ORA;
    ISA_table[0x09].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0x0D].opcode = opcode_ORA;
    ISA_table[0x0D].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0x11].opcode = opcode_ORA;
    ISA_table[0x11].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_Y_INDEXED;
    ISA_table[0x15].opcode = opcode_ORA;
    ISA_table[0x15].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0x19].opcode = opcode_ORA;
    ISA_table[0x19].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_Y_INDEXED;
    ISA_table[0x1D].opcode = opcode_ORA;
    ISA_table[0x1D].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;

    /* logical exclusive OR with Accumulator */
    ISA_table[0x41].opcode = opcode_EOR;
    ISA_table[0x41].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_X_INDEXED;
    ISA_table[0x45].opcode = opcode_EOR;
    ISA_table[0x45].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x49].opcode = opcode_EOR;
    ISA_table[0x49].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0x4D].opcode = opcode_EOR;
    ISA_table[0x4D].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0x51].opcode = opcode_EOR;
    ISA_table[0x51].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_Y_INDEXED;
    ISA_table[0x55].opcode = opcode_EOR;
    ISA_table[0x55].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0x59].opcode = opcode_EOR;
    ISA_table[0x59].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_Y_INDEXED;
    ISA_table[0x5D].opcode = opcode_EOR;
    ISA_table[0x5D].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;

    /* Jump to new location */
    ISA_table[0x4C].opcode = opcode_JMP;
    ISA_table[0x4c].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0x6C].opcode = opcode_JMP;
    ISA_table[0x6C].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT;

    /* Branch on carry clear */
    ISA_table[0x90].opcode = opcode_BCC;
    ISA_table[0x90].addressing_mode = OPCODE_ADDRESSING_MODE_RELATIVE;

    /* Branch on carry set */
    ISA_table[0xB0].opcode = opcode_BCS;
    ISA_table[0xB0].addressing_mode = OPCODE_ADDRESSING_MODE_RELATIVE;

    /* Branch on result zero */
    ISA_table[0xF0].opcode = opcode_BEQ;
    ISA_table[0xF0].addressing_mode = OPCODE_ADDRESSING_MODE_RELATIVE;

    /* Branch on result not zero */
    ISA_table[0xD0].opcode = opcode_BNE;
    ISA_table[0xD0].addressing_mode = OPCODE_ADDRESSING_MODE_RELATIVE;

    /* Branch on result not zero */
    ISA_table[0x30].opcode = opcode_BMI;
    ISA_table[0x30].addressing_mode = OPCODE_ADDRESSING_MODE_RELATIVE;

    /* Branch on result plus */
    ISA_table[0x10].opcode = opcode_BPL;
    ISA_table[0x10].addressing_mode = OPCODE_ADDRESSING_MODE_RELATIVE;

    /* Branch on overflow set */
    ISA_table[0x70].opcode = opcode_BVS;
    ISA_table[0x70].addressing_mode = OPCODE_ADDRESSING_MODE_RELATIVE;

    /* Branch on overflow clear */
    ISA_table[0x50].opcode = opcode_BVC;
    ISA_table[0x50].addressing_mode = OPCODE_ADDRESSING_MODE_RELATIVE;

    /* Compare with Accumulator*/
    ISA_table[0xC9].opcode = opcode_CMP;
    ISA_table[0xC9].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0xC5].opcode = opcode_CMP;
    ISA_table[0xC5].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0xD5].opcode = opcode_CMP;
    ISA_table[0xD5].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0xCD].opcode = opcode_CMP;
    ISA_table[0xCD].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0xDD].opcode = opcode_CMP;
    ISA_table[0xDD].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;
    ISA_table[0xD9].opcode = opcode_CMP;
    ISA_table[0xD9].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_Y_INDEXED;
    ISA_table[0xC1].opcode = opcode_CMP;
    ISA_table[0xC1].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_X_INDEXED;
    ISA_table[0xD1].opcode = opcode_CMP;
    ISA_table[0xD1].addressing_mode = OPCODE_ADDRESSING_MODE_INDIRECT_Y_INDEXED;

    /* Compare memory with Index X */
    ISA_table[0xE0].opcode = opcode_CPX;
    ISA_table[0xE0].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0xE4].opcode = opcode_CPX;
    ISA_table[0xE4].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0xEC].opcode = opcode_CPX;
    ISA_table[0xEC].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;

    /* Compare memory with Index Y */
    ISA_table[0xC0].opcode = opcode_CPY;
    ISA_table[0xC0].addressing_mode = OPCODE_ADDRESSING_MODE_IMMEDIATE;
    ISA_table[0xC4].opcode = opcode_CPY;
    ISA_table[0xC4].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0xCC].opcode = opcode_CPY;
    ISA_table[0xCC].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;

    /* Test bits in memory with Accumulator */
    ISA_table[0x24].opcode = opcode_BIT;
    ISA_table[0x24].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x2C].opcode = opcode_BIT;
    ISA_table[0x2C].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;

    /* Shift left one bit */
    ISA_table[0x0A].opcode = opcode_ASL;
    ISA_table[0x0A].addressing_mode = OPCODE_ADDRESSING_MODE_ACCUMULATOR;
    ISA_table[0x06].opcode = opcode_ASL;
    ISA_table[0x06].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x16].opcode = opcode_ASL;
    ISA_table[0x16].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0x0E].opcode = opcode_ASL;
    ISA_table[0x0E].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0x1E].opcode = opcode_ASL;
    ISA_table[0x1E].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;

    /* Right shift one bit */
    ISA_table[0x4A].opcode = opcode_LSR;
    ISA_table[0x4A].addressing_mode = OPCODE_ADDRESSING_MODE_ACCUMULATOR;
    ISA_table[0x46].opcode = opcode_LSR;
    ISA_table[0x46].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x56].opcode = opcode_LSR;
    ISA_table[0x56].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0x4E].opcode = opcode_LSR;
    ISA_table[0x4E].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0x5E].opcode = opcode_LSR;
    ISA_table[0x5E].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;

    /* Rotate one bit left */
    ISA_table[0x2A].opcode = opcode_ROL;
    ISA_table[0x2A].addressing_mode = OPCODE_ADDRESSING_MODE_ACCUMULATOR;
    ISA_table[0x26].opcode = opcode_ROL;
    ISA_table[0x26].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x36].opcode = opcode_ROL;
    ISA_table[0x36].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0x2E].opcode = opcode_ROL;
    ISA_table[0x2E].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0x3E].opcode = opcode_ROL;
    ISA_table[0x3E].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;

    /* Rotate one bit right */
    ISA_table[0x6A].opcode = opcode_ROR;
    ISA_table[0x6A].addressing_mode = OPCODE_ADDRESSING_MODE_ACCUMULATOR;
    ISA_table[0x66].opcode = opcode_ROR;
    ISA_table[0x66].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
    ISA_table[0x76].opcode = opcode_ROR;
    ISA_table[0x76].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED;
    ISA_table[0x6E].opcode = opcode_ROR;
    ISA_table[0x6E].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;
    ISA_table[0x7E].opcode = opcode_ROR;
    ISA_table[0x7E].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED;

    /* Transfer Accumulator to Index X */
    ISA_table[0xAA].opcode = opcode_TAX;
    ISA_table[0xAA].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Transfer Accumulator to Index Y */
    ISA_table[0xA8].opcode = opcode_TAY;
    ISA_table[0xA8].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Transfer Index X to Accumulator */
    ISA_table[0x8A].opcode = opcode_TXA;
    ISA_table[0x8A].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Transfer Index Y to Accumulator */
    ISA_table[0x98].opcode = opcode_TYA;
    ISA_table[0x98].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Transfer stack pointer to Index X */
    ISA_table[0xBA].opcode = opcode_TSX;
    ISA_table[0xBA].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Transfer Index X to stack register */
    ISA_table[0x9A].opcode = opcode_TXS;
    ISA_table[0x9A].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Push Accumulator onto stack */
    ISA_table[0x48].opcode = opcode_PHA;
    ISA_table[0x48].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Push processor status onto stack */
    ISA_table[0x08].opcode = opcode_PHP;
    ISA_table[0x08].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Pull Accumulator from stack */
    ISA_table[0x68].opcode = opcode_PLA;
    ISA_table[0x68].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Pull processor status from stack */
    ISA_table[0x28].opcode = opcode_PLP;
    ISA_table[0x28].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Jump to new location saving return address */
    ISA_table[0x20].opcode = opcode_JSR;
    ISA_table[0x20].addressing_mode = OPCODE_ADDRESSING_MODE_ABSOLUTE;

    /* Return from subroutine */
    ISA_table[0x60].opcode = opcode_RTS;
    ISA_table[0x60].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Return from interrupt */
    ISA_table[0x40].opcode = opcode_RTI;
    ISA_table[0x40].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Clear carry flag */
    ISA_table[0x18].opcode = opcode_CLC;
    ISA_table[0x18].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Clear decimal mode */
    ISA_table[0xD8].opcode = opcode_CLD;
    ISA_table[0xD8].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Clear interrupt disable bit */
    ISA_table[0x58].opcode = opcode_CLI;
    ISA_table[0x58].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Clear overflow bit */
    ISA_table[0xB8].opcode = opcode_CLV;
    ISA_table[0xB8].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Set carry flag */
    ISA_table[0x38].opcode = opcode_SEC;
    ISA_table[0x38].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Set decimal flag */
    ISA_table[0xF8].opcode = opcode_SED;
    ISA_table[0xF8].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* Set interrupt disable status */
    ISA_table[0x78].opcode = opcode_SEI;
    ISA_table[0x78].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    /* No operation */
    ISA_table[0xEA].opcode = opcode_NOP;
    ISA_table[0xEA].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    ISA_table[0x02].opcode = opcode_NOP;
    ISA_table[0x02].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    ISA_table[0x77].opcode = opcode_NOP;
    ISA_table[0x77].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;

    ISA_table[0x9C].opcode = opcode_NOP;
    ISA_table[0x9C].addressing_mode = OPCODE_ADDRESSING_MODE_IMPLIED;


    /* WTF */
    ISA_table[0x04].opcode = opcode_TSB;
    ISA_table[0x04].addressing_mode = OPCODE_ADDRESSING_MODE_ZERO_PAGE;
}

/******************************************************************************
 * Instruction set implementation
 *
 * These functions exist to wrapper the logical instruction (e.g., ADD, ROR,
 * MOV etc) with the necessary clock steps and memory access methods to make
 * the emulation cycle accurate. Actual op-code logic is implemented in
 * mos6507-microcode.c which has no knowledge of the memory map beyond what is
 * actually present in the CPU itself.
 *****************************************************************************/

int opcode_ILL(int cycle, addressing_mode_t address_mode)
{
    /* Halt and catch fire!! */
    return 0;
}

int opcode_ADC(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bal, bah, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_ADC(data);
    END_OPCODE()
    return 0;
}

int opcode_AND(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bal, bah, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_AND(data);
    END_OPCODE()
    return 0;
}

int opcode_ASL(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    if (OPCODE_ADDRESSING_MODE_ACCUMULATOR == address_mode) {
        switch(cycle) {
            case 0:
                /* Consume clock cycle for fetching op-code */
                return -1;
            case 1:
                mos6507_ASL_Accumulator();
                /* Intentional fall-through */
            default:
                /* End of op-code execution */
                break;
        }
        END_OPCODE()
        return 0;
    }

    FETCH_DATA()
    mos6507_ASL(&data);
    END_OPCODE()
    return 0;
}

int opcode_BCC(int cycle, addressing_mode_t address_mode)
{
    static uint8_t condition, offset = 0;
    static uint16_t addr = 0;
    uint8_t compliment = 0;

    condition = !mos6507_get_status_flag(MOS6507_STATUS_FLAG_CARRY);

    CALC_BRANCH()

    mos6507_set_address_bus(mos6507_get_PC());
    return 0;
}

int opcode_BCS(int cycle, addressing_mode_t address_mode)
{
    static uint8_t condition, offset = 0;
    static uint16_t addr = 0;
    uint8_t compliment = 0;

    condition = mos6507_get_status_flag(MOS6507_STATUS_FLAG_CARRY);

    CALC_BRANCH()

    mos6507_set_address_bus(mos6507_get_PC());
    return 0;
}

int opcode_BEQ(int cycle, addressing_mode_t address_mode)
{
    static uint8_t condition, offset = 0;
    static uint16_t addr = 0;
    uint8_t compliment = 0;

    condition = mos6507_get_status_flag(MOS6507_STATUS_FLAG_ZERO);

    CALC_BRANCH()

    mos6507_set_address_bus(mos6507_get_PC());
    return 0;
}

int opcode_BIT(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_BIT(data);
    END_OPCODE()
    return 0;
}

int opcode_BMI(int cycle, addressing_mode_t address_mode)
{
    static uint8_t condition, offset = 0;
    static uint16_t addr = 0;
    uint8_t compliment = 0;

    condition = mos6507_get_status_flag(MOS6507_STATUS_FLAG_NEGATIVE);

    CALC_BRANCH()

    mos6507_set_address_bus(mos6507_get_PC());
    return 0;
}

int opcode_BNE(int cycle, addressing_mode_t address_mode)
{
    static uint8_t condition, offset = 0;
    static uint16_t addr = 0;
    uint8_t compliment = 0;

    condition = !mos6507_get_status_flag(MOS6507_STATUS_FLAG_ZERO);

    CALC_BRANCH()

    mos6507_set_address_bus(mos6507_get_PC());
    return 0;
}

int opcode_BPL(int cycle, addressing_mode_t address_mode)
{
    static uint8_t condition, offset = 0;
    static uint16_t addr = 0;
    uint8_t compliment = 0;

    condition = !mos6507_get_status_flag(MOS6507_STATUS_FLAG_NEGATIVE);

    CALC_BRANCH()

    mos6507_set_address_bus(mos6507_get_PC());
    return 0;
}

int opcode_BRK(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, pcl, pch, S, P = 0;

    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_increment_PC();
            return -1;
        case 2:
            pch = (uint8_t)(mos6507_get_PC() >> 8);
            mos6507_push_stack(pch);
            return -1;
        case 3:
            pcl = (uint8_t)mos6507_get_PC();
            mos6507_push_stack(pcl);
            return -1;
        case 4:
            mos6507_get_register(MOS6507_REG_P, &P);
            mos6507_push_stack(P);
            return -1;
        case 5:
            mos6507_set_address_bus(0xFFFE);
            memmap_read(&adl);
            return -1;
        case 6:
            mos6507_set_address_bus(0xFFFF);
            memmap_read(&adh);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    mos6507_set_PC_hl(adh, adl);
    mos6507_set_address_bus(mos6507_get_PC());

    return 0;
}

int opcode_BVC(int cycle, addressing_mode_t address_mode)
{
    static uint8_t condition, offset = 0;
    static uint16_t addr = 0;
    uint8_t compliment = 0;

    condition = !mos6507_get_status_flag(MOS6507_STATUS_FLAG_OVERFLOW);

    CALC_BRANCH()

    mos6507_set_address_bus(mos6507_get_PC());
    return 0;
}

int opcode_BVS(int cycle, addressing_mode_t address_mode)
{
    static uint8_t condition, offset = 0;
    static uint16_t addr = 0;
    uint8_t compliment = 0;

    condition = mos6507_get_status_flag(MOS6507_STATUS_FLAG_OVERFLOW);

    CALC_BRANCH()

    END_OPCODE()
    return 0;
}

int opcode_CLC(int cycle, addressing_mode_t address_mode)
{
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, 0);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_CLD(int cycle, addressing_mode_t address_mode)
{
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_DECIMAL, 0);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_CLI(int cycle, addressing_mode_t address_mode)
{
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_INTERRUPT, 0);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_CLV(int cycle, addressing_mode_t address_mode)
{
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_OVERFLOW, 0);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_CMP(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bal, bah, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_CMP(data);
    END_OPCODE()
    return 0;
}

int opcode_CPX(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_CPX(data);
    END_OPCODE()
    return 0;
}

int opcode_CPY(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_CPY(data);
    END_OPCODE()
    return 0;
}

int opcode_DEC(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA();
    data--;
    mos6507_set_data_bus(data);
    memmap_write();
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(data & 0xFF));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (data & 0x80));
    END_OPCODE()
    return 0;
}

int opcode_DEX(int cycle, addressing_mode_t address_mode)
{
    static uint8_t value = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_get_register(MOS6507_REG_X, &value);
            value--;
            mos6507_set_register(MOS6507_REG_X, value);
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(value & 0xFF));
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (value & 0x80));
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_DEY(int cycle, addressing_mode_t address_mode)
{
    static uint8_t value = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_get_register(MOS6507_REG_Y, &value);
            value--;
            mos6507_set_register(MOS6507_REG_Y, value);
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(value & 0xFF));
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (value & 0x80));
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_EOR(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_EOR(data);
    END_OPCODE()
    return 0;
}

int opcode_INC(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA();
    data++;
    mos6507_set_data_bus(data);
    memmap_write();
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(data & 0xFF));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (data & 0x80));
    END_OPCODE()
    return 0;
}

int opcode_INX(int cycle, addressing_mode_t address_mode)
{
    uint8_t value = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_get_register(MOS6507_REG_X, &value);
            value++;
            mos6507_set_register(MOS6507_REG_X, value);
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(value & 0xFF));
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (value & 0x80));
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_INY(int cycle, addressing_mode_t address_mode)
{
    uint8_t value = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_get_register(MOS6507_REG_Y, &value);
            value++;
            mos6507_set_register(MOS6507_REG_Y, value);
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(value & 0xFF));
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (value & 0x80));
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_JMP(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            memmap_read(&adl);
            return -1;
        case 2:
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            memmap_read(&adh);
            return -1;
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    mos6507_set_PC_hl(adh, adl);
    mos6507_set_address_bus_hl(adh, adl);
    return 0;
}

int opcode_JSR(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, pcl, pch, S = 0;
    uint16_t address = 0;

    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            memmap_read(&adl);
            return -1;
        case 2:
            mos6507_get_register(MOS6507_REG_S, &S);
            mos6507_set_address_bus_hl(STACK_PAGE, S);
            return -1;
        case 3:
            pch = (uint8_t)(mos6507_get_PC() >> 8);
            mos6507_push_stack(pch);
            return -1;
        case 4:
            pcl = (uint8_t)mos6507_get_PC();
            mos6507_push_stack(pcl);
            return -1;
        case 5:
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            memmap_read(&adh);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }
    mos6507_set_address_bus_hl(adh, adl);
    mos6507_set_PC_hl(adh, adl);

    return 0;
}

int opcode_LDA(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()

    mos6507_set_register(MOS6507_REG_A, data);
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(data & 0xFF));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (data & 0x80));
    END_OPCODE()
    return 0;
}

int opcode_LDX(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_set_register(MOS6507_REG_X, data);
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(data & 0xFF));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (data & 0x80));
    END_OPCODE()
    return 0;
}

int opcode_LDY(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_set_register(MOS6507_REG_Y, data);
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(data & 0xFF));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (data & 0x80));
    END_OPCODE()
    return 0;
}

int opcode_LSR(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    if (OPCODE_ADDRESSING_MODE_ACCUMULATOR == address_mode) {
        switch(cycle) {
            case 0:
                /* Consume clock cycle for fetching op-code */
                return -1;
            case 1:
                mos6507_LSR_Accumulator();
                /* Intentional fall-through */
            default:
                /* End of op-code execution */
                break;
        }
        END_OPCODE()
        return 0;
    }

    FETCH_DATA()
    mos6507_LSR(&data);
    END_OPCODE()
    return 0;
}

int opcode_NOP(int cycle, addressing_mode_t address_mode)
{
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1: ;
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }
    END_OPCODE()
    return 0;
}


int opcode_ORA(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bal, bah, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_ORA(&data);
    END_OPCODE()
    return 0;
}


int opcode_TSB(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bal, bah, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_TSB(&data);
    END_OPCODE()
    return 0;
}

int opcode_PHA(int cycle, addressing_mode_t address_mode)
{
    static uint8_t value, S = 0;

    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            /* Consume another clock cycle incrementing PC */
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            return -1;
        case 2:
            /* Fetch value of Accumulator register and stack pointer */
            mos6507_get_register(MOS6507_REG_A, &value);
            mos6507_push_stack(value);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }
    mos6507_set_address_bus(mos6507_get_PC());

    return 0;
}

int opcode_PHP(int cycle, addressing_mode_t address_mode)
{
    static uint8_t value, S= 0;

    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            /* Consume another clock cycle incrementing PC */
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            return -1;
        case 2:
            /* Fetch value of status register and stack pointer */
            mos6507_get_register(MOS6507_REG_P, &value);
            mos6507_push_stack(value);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    mos6507_set_address_bus(mos6507_get_PC());
    return 0;
}

int opcode_PLA(int cycle, addressing_mode_t address_mode)
{
    static uint8_t value, source = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            /* Consume another clock cycle incrementing PC */
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            return -1;
        case 2:
            mos6507_get_register(MOS6507_REG_S, &source);
            mos6507_set_address_bus_hl(STACK_PAGE, source);
            return -1;
        case 3:
            mos6507_pull_stack(&value);
            mos6507_set_register(MOS6507_REG_A, value);
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (value & 0x80));
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(value & 0xFF));
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    mos6507_set_address_bus(mos6507_get_PC());
    return 0;
}

int opcode_PLP(int cycle, addressing_mode_t address_mode)
{
    static uint8_t value, source = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            /* Consume another clock cycle incrementing PC */
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            return -1;
        case 2:
            mos6507_get_register(MOS6507_REG_S, &source);
            mos6507_set_address_bus_hl(STACK_PAGE, source);
            return -1;
        case 3:
            mos6507_pull_stack(&value);
            mos6507_set_register(MOS6507_REG_P, value);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    mos6507_set_address_bus(mos6507_get_PC());
    return 0;
}

int opcode_ROL(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bal, bah, data = 0;
    uint8_t X, Y, c = 0;

    if (OPCODE_ADDRESSING_MODE_ACCUMULATOR == address_mode) {
        switch(cycle) {
            case 0:
                /* Consume clock cycle for fetching op-code */
                return -1;
            case 1:
                mos6507_ROL_Accumulator();
                /* Intentional fall-through */
            default:
                /* End of op-code execution */
                break;
        }
        END_OPCODE()
        return 0;
    }

    FETCH_DATA()
    mos6507_ROL(&data);
    END_OPCODE()
    return 0;
}

int opcode_ROR(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    if (OPCODE_ADDRESSING_MODE_ACCUMULATOR == address_mode) {
        switch(cycle) {
            case 0:
                /* Consume clock cycle for fetching op-code */
                return -1;
            case 1:
                mos6507_ROR_Accumulator();
                /* Intentional fall-through */
            default:
                /* End of op-code execution */
                break;
        }
        END_OPCODE()
        return 0;
    }

    FETCH_DATA()
    mos6507_ROR(&data);
    END_OPCODE()
    return 0;
}

int opcode_RTI(int cycle, addressing_mode_t address_mode)
{
    static uint8_t pcl, pch, S, nuS = 0;

    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            return -1;
        case 2:
            mos6507_get_register(MOS6507_REG_S, &S);
            mos6507_set_address_bus_hl(STACK_PAGE, S);
            return -1;
        case 3:
            mos6507_pull_stack(&nuS);
            mos6507_set_register(MOS6507_REG_S, nuS);
            return -1;
        case 4:
            mos6507_pull_stack(&pcl);
            return -1;
        case 5:
            mos6507_pull_stack(&pch);
            mos6507_set_PC_hl(pch, pcl);
            mos6507_set_address_bus_hl(pch, pcl);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }
    return 0;
}

int opcode_RTS(int cycle, addressing_mode_t address_mode)
{
    static uint8_t pcl, pch, S = 0;

    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            return -1;
        case 2:
            mos6507_get_register(MOS6507_REG_S, &S);
            mos6507_set_address_bus_hl(STACK_PAGE, S);
            return -1;
        case 3:
            mos6507_pull_stack(&pcl);
            return -1;
        case 4:
            mos6507_pull_stack(&pch);
            return -1;
        case 5:
            mos6507_set_PC_hl(pch, pcl);
            mos6507_set_address_bus_hl(pch, pcl);
            // TODO: Review if this is actually necessary for maintaining 
            // subroutine consistency
            mos6507_increment_PC();
            mos6507_set_address_bus(mos6507_get_PC());
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_SBC(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, ial, bal, bah, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_DATA()
    mos6507_SBC(data);
    END_OPCODE()
    return 0;
}

int opcode_SEC(int cycle, addressing_mode_t address_mode)
{
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, 1);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_SED(int cycle, addressing_mode_t address_mode)
{
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_DECIMAL, 1);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_SEI(int cycle, addressing_mode_t address_mode)
{
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_INTERRUPT, 1);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_STA(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_STORE_ADDRESS()
    mos6507_get_register(MOS6507_REG_A, &data);
    mos6507_set_data_bus(data);
    memmap_write();
    END_OPCODE()
    return 0;
}

int opcode_STX(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_STORE_ADDRESS()
    mos6507_get_register(MOS6507_REG_X, &data);
    mos6507_set_data_bus(data);
    memmap_write();
    END_OPCODE()
    return 0;
}

int opcode_STY(int cycle, addressing_mode_t address_mode)
{
    static uint8_t adl, adh, bah, bal, data = 0;
    uint8_t X, Y, c = 0;

    FETCH_STORE_ADDRESS()
    mos6507_get_register(MOS6507_REG_Y, &data);
    mos6507_set_data_bus(data);
    memmap_write();
    END_OPCODE()
    return 0;
}

int opcode_TAX(int cycle, addressing_mode_t address_mode)
{
    uint8_t value = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_get_register(MOS6507_REG_A, &value);
            mos6507_set_register(MOS6507_REG_X, value);
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(value & 0xFF));
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (value & 0x80));
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_TAY(int cycle, addressing_mode_t address_mode)
{
    uint8_t value = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_get_register(MOS6507_REG_A, &value);
            mos6507_set_register(MOS6507_REG_Y, value);
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(value & 0xFF));
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (value & 0x80));
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_TSX(int cycle, addressing_mode_t address_mode)
{
    uint8_t value = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_get_register(MOS6507_REG_S, &value);
            mos6507_set_register(MOS6507_REG_X, value);
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(value & 0xFF));
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (value & 0x80));
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_TXA(int cycle, addressing_mode_t address_mode)
{
    uint8_t value = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_get_register(MOS6507_REG_X, &value);
            mos6507_set_register(MOS6507_REG_A, value);
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(value & 0xFF));
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (value & 0x80));
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_TXS(int cycle, addressing_mode_t address_mode)
{
    uint8_t value = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_get_register(MOS6507_REG_X, &value);
            mos6507_set_register(MOS6507_REG_S, value);
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

int opcode_TYA(int cycle, addressing_mode_t address_mode)
{
    uint8_t value = 0;
    switch(cycle) {
        case 0:
            /* Consume clock cycle for fetching op-code */
            return -1;
        case 1:
            mos6507_get_register(MOS6507_REG_Y, &value);
            mos6507_set_register(MOS6507_REG_A, value);
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(value & 0xFF));
            mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (value & 0x80));
            /* Intentional fall-through */
        default:
            /* End of op-code execution */
            break;
    }

    END_OPCODE()
    return 0;
}

