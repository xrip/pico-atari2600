/*
 * File: mos6507-microcode.c
 * Author: dgrubb
 * Date: 06/27/2017
 *
 * Provides implementation of CPU logic without reference to
 * specific addressing modes.
 */

#include "mos6507-microcode.h"
#include "mos6507.h"

/* Add memory to Accumulator.
 * A + M + C -> A, C
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - + |
 */
void mos6507_ADC(uint8_t data)
{
    uint16_t tmp = 0;
    uint8_t accumulator = 0;
    mos6507_get_register(MOS6507_REG_A, &accumulator);

    if (mos6507_get_status_flag(MOS6507_STATUS_FLAG_DECIMAL)) {
        /* Interesting! According to Bill Mensch of MOS Technologies this
         * feature did actually get used by Atari for their port of Asteroids.
         * https://www.youtube.com/watch?v=Ne1ApyqSvm0 (55:00)
         */
    } else {
        tmp = data + accumulator + (mos6507_get_status_flag(MOS6507_STATUS_FLAG_CARRY) ? 1 : 0);
        mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
        mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, (tmp > 0xFF));
        mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
        mos6507_set_status_flag(
            MOS6507_STATUS_FLAG_OVERFLOW,
            !((accumulator ^ data) & 0x80) && ((accumulator ^ tmp) & 0x80)
        );
    }

    mos6507_set_register(MOS6507_REG_A, (tmp & 0xFF));
}

/* Logical AND memory with Accumulator.
 * A & M -> A
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + - - - - |
 */
void mos6507_AND(uint8_t data)
{
    uint8_t accumulator, tmp = 0;
    mos6507_get_register(MOS6507_REG_A, &accumulator);

    tmp = accumulator & data;
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));

    mos6507_set_register(MOS6507_REG_A, tmp);
}

/* Shift left by one bit.
 * C <- [76543210] <- 0
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - - |
 */
void mos6507_ASL(uint8_t *data)
{
    uint16_t tmp = *data;

    tmp <<= 1;
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, (tmp & 0x100));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));

    *data = (tmp & 0xFF);
}

/* Shift Accumulator left by one bit.
 * C <- [76543210] <- 0
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - - |
 */
void mos6507_ASL_Accumulator(void)
{
    uint8_t accumulator;
    uint16_t tmp;
    mos6507_get_register(MOS6507_REG_A, &accumulator);
    tmp = accumulator;

    tmp <<= 1;
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, (tmp & 0x100));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));

    accumulator = (tmp & 0xFF);
    mos6507_set_register(MOS6507_REG_A, accumulator);
}

/* Test bits in memory with Accumulator. Bits 7 and 6 of operand are transfered
 * to bit 7 and 6 of status register (N, V). Zeroflag is set to the result of
 * operand & Accumulator.
 * A & M, M[7] -> N, M[6] -> V
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | 7 + - - - 6 |
 */
void mos6507_BIT(uint8_t data)
{
    uint8_t accumulator, tmp = 0;
    mos6507_get_register(MOS6507_REG_A, &accumulator);

    tmp = accumulator & data;

    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (data & MOS6507_STATUS_FLAG_NEGATIVE));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_OVERFLOW, (data & MOS6507_STATUS_FLAG_OVERFLOW));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
}

/* Compare memory with Accumulator.
 * A - M
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - - |
 */
void mos6507_CMP(uint8_t data)
{
    uint16_t tmp = 0;
    uint8_t accumulator = 0;

    mos6507_get_register(MOS6507_REG_A, &accumulator);

    tmp = accumulator - data;
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, (tmp < 0x0100));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
}

/* Compare memory with Index X.
 * X - M
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - - |
 */
void mos6507_CPX(uint8_t data)
{
    uint16_t tmp = 0;
    uint8_t X = 0;

    mos6507_get_register(MOS6507_REG_X, &X);

    tmp = X - data;
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, (tmp < 0x0100));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
}

/* Compare memory with Index Y.
 * Y - M
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - - |
 */
void mos6507_CPY(uint8_t data)
{
    uint16_t tmp = 0;
    uint8_t Y = 0;

    mos6507_get_register(MOS6507_REG_Y, &Y);

    tmp = Y - data;
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, (tmp < 0x0100));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
}

/* Exclusive OR memory with Accumulator.
 * A ^ M -> A
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + - - - - |
 */
void mos6507_EOR(uint8_t data)
{
    uint8_t accumulator, tmp = 0;
    mos6507_get_register(MOS6507_REG_A, &accumulator);

    tmp = accumulator ^ data;

    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
    mos6507_set_register(MOS6507_REG_A, tmp);
}

/* Shift right by one bit.
 * 0 -> [76543210] -> C
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | - + + - - - |
 */
void mos6507_LSR(uint8_t *data)
{
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, (*data & 0x01));
    *data >>= 1;
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(*data & 0xFF));
}

/* Shift right by one bit in the Accumulator.
 * 0 -> [76543210] -> C
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | - + + - - - |
 */
void mos6507_LSR_Accumulator(void)
{
    uint8_t accumulator;
    mos6507_get_register(MOS6507_REG_A, &accumulator);
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, (accumulator & 0x01));
    accumulator >>= 1;
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(accumulator & 0xFF));
    mos6507_set_register(MOS6507_REG_A, accumulator);
}

/* Logical OR memory with Accumulator.
 * A | M -> A
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + - - - - |
 */
void mos6507_ORA(uint8_t *data)
{
    uint8_t accumulator, tmp = 0;
    mos6507_get_register(MOS6507_REG_A, &accumulator);

    tmp = accumulator | *data;

    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
    mos6507_set_register(MOS6507_REG_A, tmp);
}


// WTF
void mos6507_TSB(uint8_t *data)
{
    uint8_t accumulator, tmp = 0;
    mos6507_get_register(MOS6507_REG_A, &accumulator);

    tmp = accumulator | *data;

//    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, (*data & tmp));
    mos6507_set_register(MOS6507_REG_A, tmp);
}

/* Rotate one bit left.
 * C <- [76543210] <- C
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - - |
 */
void mos6507_ROL(uint8_t *data)
{
    uint16_t tmp = 0;

    tmp = *data << 1;
    if (mos6507_get_status_flag(MOS6507_STATUS_FLAG_CARRY)) {
        tmp |= 0x01;
    }
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, (tmp & 0x0100));
    *data = (tmp & 0xFF);
}

/* Rotate Accumulator one bit left.
 * C <- [76543210] <- C
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - - |
 */
void mos6507_ROL_Accumulator(void)
{
    uint16_t tmp = 0;
    uint8_t accumulator;
    mos6507_get_register(MOS6507_REG_A, &accumulator);

    tmp = accumulator << 1;
    if (mos6507_get_status_flag(MOS6507_STATUS_FLAG_CARRY)) {
        tmp |= 0x01;
    }
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, (tmp & 0x0100));
    accumulator = (tmp & 0xFF);
    mos6507_set_register(MOS6507_REG_A, accumulator);
}

/* Rotate one bit right.
 * C -> [76543210] -> C
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - - |
 */
void mos6507_ROR(uint8_t *data)
{
    uint16_t tmp = 0;
    uint8_t tmpCarry = 0;

    tmpCarry = (*data & 0x01);
    tmp = *data >> 1;
    if (mos6507_get_status_flag(MOS6507_STATUS_FLAG_CARRY)) {
        tmp |= 0x80;
    }
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, tmpCarry);
    *data = (tmp & 0xFF);
}

/* Rotate Accumulator one bit right.
 * C -> [76543210] -> C
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - - |
 */
void mos6507_ROR_Accumulator(void)
{
    uint16_t tmp, tmpCarry = 0;
    uint8_t accumulator;
    mos6507_get_register(MOS6507_REG_A, &accumulator);

    tmpCarry = accumulator & 0x01;
    tmp = accumulator >> 1;
    if (mos6507_get_status_flag(MOS6507_STATUS_FLAG_CARRY)) {
        tmp |= 0x80;
    }
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
    mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, tmpCarry);
    accumulator = (tmp & 0xFF);
    mos6507_set_register(MOS6507_REG_A, accumulator);
}
/* Subtract memory from Accumulator with borrow.
 * A - M - C -> A
 *
 * Status flag changes (+ = conditionally modified, 1 = set, 0 = cleared):
 *
 * | N Z C I D V |
 * | + + + - - + |
 */
void mos6507_SBC(uint8_t data)
{
    uint16_t tmp = 0;
    uint8_t accumulator = 0;
    mos6507_get_register(MOS6507_REG_A, &accumulator);

    if (mos6507_get_status_flag(MOS6507_STATUS_FLAG_DECIMAL)) {
        // TODO
    } else {
        tmp = accumulator - data - (mos6507_get_status_flag(MOS6507_STATUS_FLAG_CARRY) ? 1 : 0);
        mos6507_set_status_flag(MOS6507_STATUS_FLAG_NEGATIVE, (tmp & 0x80));
        mos6507_set_status_flag(MOS6507_STATUS_FLAG_CARRY, !(tmp & 0x8000));
        mos6507_set_status_flag(MOS6507_STATUS_FLAG_ZERO, !(tmp & 0xFF));
        mos6507_set_status_flag(
            MOS6507_STATUS_FLAG_OVERFLOW,
            !((accumulator ^ data) & 0x80) && ((accumulator ^ tmp) & 0x80)
        );
    }

    mos6507_set_register(MOS6507_REG_A, (tmp & 0xFF));
}
