/*
 * File: Atari-TIA.h
 * Author: dgrubb
 * Date: 09/18/2017
 *
 * Implements the TIA memory map.
 */

#ifndef _ATARI_TIA_H
#define _ATARI_TIA_H

#include <stdint.h>

#define X4(a) (a | (a << 8) | (a << 16) | (a << 24))
#define VGA_RGB_222(r, g, b) ((r << 4) | (g << 2) | b)

/* Ref: Stella Programmer's Guide, Pg. 4 */
#define TIA_COLOUR_CLOCK_VISIBLE    160
#define TIA_COLOUR_CLOCK_VISIBLE_HALF (TIA_COLOUR_CLOCK_VISIBLE / 2)
#define TIA_COLOUR_CLOCK_HSYNC      68
#define TIA_COLOUR_CLOCK_TOTAL      (TIA_COLOUR_CLOCK_VISIBLE + TIA_COLOUR_CLOCK_HSYNC)

#define TIA_VERTICAL_PICTURE_LINES  192
#define TIA_VERTICAL_SYNC_LINES     3
#define TIA_VERTICAL_BLANK_LINES    37
#define TIA_VERTICAL_OVERSCAN_LINES 30
#define TIA_VERTICAL_TOTAL_LINES    (TIA_VERTICAL_PICTURE_LINES + \
                                     TIA_VERTICAL_SYNC_LINES + \
                                     TIA_VERTICAL_BLANK_LINES + \
                                     TIA_VERTICAL_OVERSCAN_LINES)

/* Define available memory registers semantically */
/* Writable registers */
typedef enum {
    TIA_WRITE_REG_VSYNC = 0x00, /* Veritical sync */
    TIA_WRITE_REG_VBLANK, /* 0x01: Vertical blank clear */
    TIA_WRITE_REG_WSYNC,  /* 0x02: Wait for horizontal blank */
    TIA_WRITE_REG_RSYNC,  /* 0x03: Reset horizontal sync counter */
    TIA_WRITE_REG_NUSIZ0, /* 0x04: Number size, player missile 0 */
    TIA_WRITE_REG_NUSIZ1, /* 0x05: Number size, player missile 1 */
    TIA_WRITE_REG_COLUP0, /* 0x06: Colour lumenance, player 0 */
    TIA_WRITE_REG_COLUP1, /* 0x07: Colour lumenance, player 1 */
    TIA_WRITE_REG_COLUPF, /* 0x08: Colour lumenance, playfield */
    TIA_WRITE_REG_COLUBK, /* 0x09: Colour lumenance, background */
    TIA_WRITE_REG_CTRLPF, /* 0x0A: Playfield size and collision control */
    TIA_WRITE_REG_REFP0,  /* 0x0B: Reflect player 0 */
    TIA_WRITE_REG_REFP1,  /* 0x0C: Reflect player 1 */
    TIA_WRITE_REG_PF0,    /* 0x0D: Playfield register byte 0 */
    TIA_WRITE_REG_PF1,    /* 0x0E: Playfield register byte 1 */
    TIA_WRITE_REG_PF2,    /* 0x0F: Playfield register byte 2 */
    TIA_WRITE_REG_RESP0,  /* 0x10: Reset player 0 */
    TIA_WRITE_REG_RESP1,  /* 0x11: Reset player 1 */
    TIA_WRITE_REG_RESM0,  /* 0x12: Reset missile 0 */
    TIA_WRITE_REG_RESM1,  /* 0x13: Reset missile 1 */
    TIA_WRITE_REG_RESBL,  /* 0x14: Reset ball */
    TIA_WRITE_REG_AUDC0,  /* 0x15: Audio control 0 */
    TIA_WRITE_REG_AUDC1,  /* 0x16: Audio control 1 */
    TIA_WRITE_REG_AUDF0,  /* 0x17: Audio frequency 0 */
    TIA_WRITE_REG_AUDF1,  /* 0x18: Audio frequency 1 */
    TIA_WRITE_REG_AUDV0,  /* 0x19: Audio volume 0 */
    TIA_WRITE_REG_AUDV1,  /* 0x1A: Audio volume 1 */
    TIA_WRITE_REG_GRP0,   /* 0x1B: Graphics player 0 */
    TIA_WRITE_REG_GRP1,   /* 0x1C: Graphics player 1 */
    TIA_WRITE_REG_ENAM0,  /* 0x1D: Enable missile 0 graphics */
    TIA_WRITE_REG_ENAM1,  /* 0x1E: Enable missile 1 graphics */
    TIA_WRITE_REG_ENABL,  /* 0x1F: Enable ball graphics */
    TIA_WRITE_REG_HMP0,   /* 0x20: Horizontal motion player 0 */
    TIA_WRITE_REG_HMP1,   /* 0x21: Horizontal motion player 1 */
    TIA_WRITE_REG_HMM0,   /* 0x22: Horizontal motion missile 0 */
    TIA_WRITE_REG_HMM1,   /* 0x23: Horizontal motion missile 1 */
    TIA_WRITE_REG_HMBL,   /* 0x24: Horizontal ball motion */
    TIA_WRITE_REG_VDELP0, /* 0x25: Vertical delay player 0 */
    TIA_WRITE_REG_VDELP1, /* 0x26: Vertical delay player 1 */
    TIA_WRITE_REG_VDELBL, /* 0x27: Vertical delay ball */
    TIA_WRITE_REG_RESMP0, /* 0x28: Reset missile 0 to player 0 */
    TIA_WRITE_REG_RESMP1, /* 0x29: Reset missile 1 to player 1 */
    TIA_WRITE_REG_HMOVE,  /* 0x2A: Apply horizontal motion */
    TIA_WRITE_REG_HMCLR,  /* 0x2B: Clear horizontal motion registers */
    TIA_WRITE_REG_CXCLR,  /* 0x2C: Clear collision latches */
    TIA_WRITE_REG_LEN
} tia_writable_register_t;

/* Readable registers */
typedef enum {
    TIA_READ_REG_CXM0P = 0x00, /* Collision - D7: M0 P1, D6: M0 P0 */
    TIA_READ_REG_CXM1P,  /* 0x01: Collision - D7: M1 P0, D6: M1 P1 */
    TIA_READ_REG_CXP0FB, /* 0x02: Collision - D7: P0 PF, D6: P0 BL */
    TIA_READ_REG_CXP1FB, /* 0x03: Collision - D7: P1 PF, D6: P1 BL */
    TIA_READ_REG_CXM0FB, /* 0x04: Collision - D7: M0 PF, D6: M0 BL */
    TIA_READ_REG_CXM1FB, /* 0x05: Collision - D7: M1 PF, D6: M1 BL */
    TIA_READ_REG_CXBLPF, /* 0x06: Collision - D7: BL PF, D6: unused */
    TIA_READ_REG_CXPPMM, /* 0x07: Collision - D7: P0 P1, D6: M0 M1 */
    TIA_READ_REG_INPT0,  /* 0x08: Read potentiometer port */
    TIA_READ_REG_INPT1,  /* 0x09: Read potentiometer port */
    TIA_READ_REG_INPT2,  /* 0x09: Read potentiometer port */
    TIA_READ_REG_INPT3,  /* 0x09: Read potentiometer port */
    TIA_READ_REG_INPT4,  /* 0x09: Read potentiometer port */
    TIA_READ_REG_INPT5,  /* 0x09: Read potentiometer port */
    TIA_READ_REG_LEN
} tia_readable_register_t;

typedef struct {
    uint8_t scanline_reset;
    uint8_t enabled;
    uint32_t position_clock;
    uint8_t width;
    int8_t horizontal_offset;
    uint8_t line_buffer[TIA_COLOUR_CLOCK_VISIBLE];
} tia_missile_t;

typedef struct {
    uint8_t scanline_reset;
    uint8_t enabled;
    uint32_t position_clock;
    uint8_t width;
    int8_t horizontal_offset;
    uint8_t vertical_delay;
    uint8_t line_buffer[TIA_COLOUR_CLOCK_VISIBLE];
} tia_ball_t;

typedef struct {
    uint8_t mirror_enable;
    uint8_t score_enabled;
    uint8_t line_buffer[TIA_COLOUR_CLOCK_VISIBLE];
} tia_playfield_t;

typedef struct {
    uint8_t scanline_reset;
    int8_t position_clock;
    int8_t horizontal_offset;
    uint8_t vertical_delay;
    uint8_t pattern;
    uint8_t line_buffer[TIA_COLOUR_CLOCK_VISIBLE];
} tia_player_t;

/* Define a structure type to represent the state of a TIA chip */
typedef struct {
    // Data registers
    uint8_t write_regs[TIA_WRITE_REG_LEN];
    uint8_t read_regs[TIA_READ_REG_LEN];
    uint32_t colour_clock;
    tia_ball_t ball;
    tia_missile_t missiles[2];
    tia_player_t players[2];
    tia_playfield_t playfield;
} atari_tia;

typedef struct {
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A;
} tia_pixel_t;

extern tia_pixel_t tia_colour_map[128];
extern uint16_t tia_player_size_map[8];

/* This is the single instance of atari_tia type to represent
 * the TIA in this application */
extern atari_tia tia;

/* To allow for easier output to non-raster devices we'll build the image one
 * line at a time into this buffer.
 */
#if PICO_ON_DEVICE
extern uint32_t tia_raw_buffer[TIA_COLOUR_CLOCK_VISIBLE];
#else
extern tia_pixel_t tia_line_buffer[TIA_COLOUR_CLOCK_VISIBLE];
#endif

// Joystic 1
void TIA_joy1_state(uint8_t state);

/* Interfacing functions */
void TIA_init(void);
void TIA_read_register(uint8_t reg, uint8_t *value);
void TIA_write_register(uint8_t reg, uint8_t value);

int TIA_clock_tick(void);
void TIA_generate_colour(void);

int TIA_get_WSYNC(void);
int TIA_get_VSYNC(void);
int TIA_get_VBLANK(void);

void TIA_reset_buffer();
void TIA_reset_line_buffer(uint8_t line_buffer[]);

uint8_t TIA_reverse_byte(uint8_t byte);


void TIA_apply_HMOVE();

// Playfield
int TIA_test_playfield_bit();
void TIA_update_playfield();

// Player #x
int TIA_test_player_bit(uint8_t player);
void TIA_reset_player(uint8_t player);
void TIA_update_player_buffer(uint8_t player);
void TIA_get_player_registers(uint8_t player, tia_writable_register_t *reflect,
        tia_writable_register_t *graphics, tia_writable_register_t *offset,
        tia_writable_register_t *vertical, tia_writable_register_t *size_reg);
void TIA_update_player_HMOVE(uint8_t player);

// Missile #x
int TIA_test_missile_bit(uint8_t missile);
void TIA_reset_missile(uint8_t missile);
void TIA_update_missile_buffer(uint8_t missile);
void TIA_get_missile_registers(uint8_t missile, tia_writable_register_t *enable,
        tia_writable_register_t *size, tia_writable_register_t *offset);
void TIA_update_missile_HMOVE(uint8_t missile);

// Ball
int TIA_test_ball_bit();
void TIA_reset_ball();
void TIA_update_ball_buffer();
void TIA_update_ball_HMOVE();

#endif /* _ATARI_TIA_H */
