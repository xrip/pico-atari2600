/*
 * File: mos6507-microcode.h
 * Author: dgrubb
 * Date: 06/27/2017
 *
 * Provides implementation of CPU logic without reference to
 * specific addressing modes.
 */

#ifndef _MOS6507_MICROCODE_H
#define _MOS6507_MICROCODE_H

#include <stdint.h>

void mos6507_ADC(uint8_t data);
void mos6507_AND(uint8_t data);
void mos6507_ASL(uint8_t *data);
void mos6507_ASL_Accumulator(void);
void mos6507_BIT(uint8_t data);
void mos6507_CMP(uint8_t data);
void mos6507_CPX(uint8_t data);
void mos6507_CPY(uint8_t data);
void mos6507_EOR(uint8_t data);
void mos6507_LSR(uint8_t *data);
void mos6507_LSR_Accumulator(void);
void mos6507_ORA(uint8_t *data);

void mos6507_ROL(uint8_t *data);
void mos6507_ROL_Accumulator(void);
void mos6507_ROR(uint8_t *data);
void mos6507_ROR_Accumulator(void);
void mos6507_SBC(uint8_t data);

void mos6507_TSB(uint8_t *data);


#endif /* _MOS6507_MICROCODE_H */

