/*
 * File: mos6507-opcodes.h
 * Author: dgrubb
 * Date: 12/20/2016
 *
 * Provides implementations of the 6507's op-codes.
 */

#ifndef _MOS6507_OPCODES_H
#define _MOS6507_OPCODES_H

#include <stdint.h>

/* Opcodes are 8-bit, allowing for 255 unique permutations.
 * However, many bit selections don't represent valid operations.
 */
#define ISA_LENGTH 255

typedef enum {
    OPCODE_ADDRESSING_MODE_ACCUMULATOR = 0,
    OPCODE_ADDRESSING_MODE_ABSOLUTE,
    OPCODE_ADDRESSING_MODE_ABSOLUTE_X_INDEXED,
    OPCODE_ADDRESSING_MODE_ABSOLUTE_Y_INDEXED,
    OPCODE_ADDRESSING_MODE_IMMEDIATE,
    OPCODE_ADDRESSING_MODE_IMPLIED,
    OPCODE_ADDRESSING_MODE_INDIRECT,
    OPCODE_ADDRESSING_MODE_INDIRECT_X_INDEXED,
    OPCODE_ADDRESSING_MODE_INDIRECT_Y_INDEXED,
    OPCODE_ADDRESSING_MODE_RELATIVE,
    OPCODE_ADDRESSING_MODE_ZERO_PAGE,
    OPCODE_ADDRESSING_MODE_ZERO_PAGE_X_INDEXED,
    OPCODE_ADDRESSING_MODE_ZERO_PAGE_Y_INDEXED,
} addressing_mode_t;

/* Define a function pointer type */
typedef int (*fp)(int, addressing_mode_t);

typedef struct {
    fp opcode;
    addressing_mode_t addressing_mode;
} instruction_t;

extern instruction_t ISA_table[ISA_LENGTH];

void opcode_populate_ISA_table(void);
int opcode_execute(uint8_t opcode);
int opcode_validate(uint8_t opcode);

/* The following function prototypes define each possible opcodes from a
 * 6507 with nmemonic annotation in commens. An additional opcode ILL is
 * added to handle illegal codes placed into a program but has no analogue
 * in a real 6507 device.
 *
 * Annotation derived from explanations found at:
 * http://www.dwheeler.com/6502/oneelkruns/asm1step.html
 */

/* Load and store */
int opcode_LDA(int cycle, addressing_mode_t address_mode); /* LoaD the Accumulator */
int opcode_LDX(int cycle, addressing_mode_t address_mode); /* LoaD the X register */
int opcode_LDY(int cycle, addressing_mode_t address_mode); /* LoaD the Y register */
int opcode_STA(int cycle, addressing_mode_t address_mode); /* STore the Accumulator */
int opcode_STX(int cycle, addressing_mode_t address_mode); /* STore the X register */
int opcode_STY(int cycle, addressing_mode_t address_mode); /* STore the Y register */

/* Arithmetic */
int opcode_ADC(int cycle, addressing_mode_t address_mode); /* ADd to Accumulator with Carry */
int opcode_SBC(int cycle, addressing_mode_t address_mode); /* SuBtract from accumulator with Carry*/

/* Increment and decrement */
int opcode_INC(int cycle, addressing_mode_t address_mode); /* INCrement memory by one */
int opcode_INX(int cycle, addressing_mode_t address_mode); /* INcrement X by one */
int opcode_INY(int cycle, addressing_mode_t address_mode); /* INcrement Y by one */
int opcode_DEC(int cycle, addressing_mode_t address_mode); /* DECrement memory by one */
int opcode_DEX(int cycle, addressing_mode_t address_mode); /* DEcrement X by one */
int opcode_DEY(int cycle, addressing_mode_t address_mode); /* DEcrement Y by one */

/* Logical */
int opcode_AND(int cycle, addressing_mode_t address_mode); /* AND memory with accumulator */
int opcode_ORA(int cycle, addressing_mode_t address_mode); /* OR memory with Accumulator */
int opcode_EOR(int cycle, addressing_mode_t address_mode); /* Exclusive-OR memory with accumulator */

/* Jump, branch, compare and test */
int opcode_JMP(int cycle, addressing_mode_t address_mode); /* JuMP to another location (GOTO) */
int opcode_BCC(int cycle, addressing_mode_t address_mode); /* Branch on Carry Clear */
int opcode_BCS(int cycle, addressing_mode_t address_mode); /* Branch on Carry Set */
int opcode_BEQ(int cycle, addressing_mode_t address_mode); /* Branch on EQual to zero */
int opcode_BNE(int cycle, addressing_mode_t address_mode); /* Branch on Not Equal to zero */
int opcode_BMI(int cycle, addressing_mode_t address_mode); /* Branch on MInus */
int opcode_BPL(int cycle, addressing_mode_t address_mode); /* Branch on PLus */
int opcode_BVS(int cycle, addressing_mode_t address_mode); /* Branch on oVerflow Set */
int opcode_BVC(int cycle, addressing_mode_t address_mode); /* Branch on oVerflow clear */
int opcode_CMP(int cycle, addressing_mode_t address_mode); /* CoMPare memory and accumulator */
int opcode_CPX(int cycle, addressing_mode_t address_mode); /* ComPare memory and X */
int opcode_CPY(int cycle, addressing_mode_t address_mode); /* ComPare memory and Y*/
int opcode_BIT(int cycle, addressing_mode_t address_mode); /* Test BITs */

/* Shift and rotate */
int opcode_ASL(int cycle, addressing_mode_t address_mode); /* Accumulator Shift Left */
int opcode_LSR(int cycle, addressing_mode_t address_mode); /* Logical Shift Right */
int opcode_ROL(int cycle, addressing_mode_t address_mode); /* ROtate Left */
int opcode_ROR(int cycle, addressing_mode_t address_mode); /* ROtate Right */

/* Transfer */
int opcode_TAX(int cycle, addressing_mode_t address_mode); /* Transfer Accumulator to X */
int opcode_TAY(int cycle, addressing_mode_t address_mode); /* Transfer Accumulator to Y */
int opcode_TXA(int cycle, addressing_mode_t address_mode); /* Transfer X to Accumulator */
int opcode_TYA(int cycle, addressing_mode_t address_mode); /* Transfer Y to Accumulator */

/* Stack */
int opcode_TSX(int cycle, addressing_mode_t address_mode); /* Transfer Stack pointer to X */
int opcode_TXS(int cycle, addressing_mode_t address_mode); /* Transfer X to Stack pointer */
int opcode_PHA(int cycle, addressing_mode_t address_mode); /* PusH Accumulator on stack */
int opcode_PHP(int cycle, addressing_mode_t address_mode); /* PusH Processor status on stack */
int opcode_PLA(int cycle, addressing_mode_t address_mode); /* PulL Accumulator from stack */
int opcode_PLP(int cycle, addressing_mode_t address_mode); /* PulL Processor status from stack */

/* Subroutine */
int opcode_JSR(int cycle, addressing_mode_t address_mode); /* Jump to SubRoutine */
int opcode_RTS(int cycle, addressing_mode_t address_mode); /* ReTurn from Subroutine */
int opcode_RTI(int cycle, addressing_mode_t address_mode); /* ReTurn from Interrupt */

/* Set and reset */
int opcode_CLC(int cycle, addressing_mode_t address_mode); /* CLear Carry flag */
int opcode_CLD(int cycle, addressing_mode_t address_mode); /* CLear Decimal mode */
int opcode_CLI(int cycle, addressing_mode_t address_mode); /* CLear Interrupt disable */
int opcode_CLV(int cycle, addressing_mode_t address_mode); /* CLear oVerflow flag */

int opcode_SEC(int cycle, addressing_mode_t address_mode); /* SEt Carry */
int opcode_SED(int cycle, addressing_mode_t address_mode); /* SEt Decimal mode */
int opcode_SEI(int cycle, addressing_mode_t address_mode); /* SEt Interrupt disable */

/* Miscellaneous */
int opcode_NOP(int cycle, addressing_mode_t address_mode); /* No OPeration */
int opcode_BRK(int cycle, addressing_mode_t address_mode); /* BReaK */

/* WTF */
int opcode_TSB(int cycle, addressing_mode_t address_mode); /* WTF */

/* This is part of the program logic rather than the 6507 model. It
 * provides a sink when illegal opcodes are invoked. Further work
 * could include fleshing out the illegal opcodes with their real-world
 * actions depending on which specific 6507 implementation is being
 * emulated?
 */
int opcode_ILL(int cycle, addressing_mode_t address_mode); /* Illegal */

#endif /* _MOS6507_OPCODES_H */
