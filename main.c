/* See LICENSE file for license details */

/* Standard library includes */
#include <stdio.h>
#include <string.h>

/* Pico libs*/
#include "pico/time.h"
#include "pico/sem.h"
#include "pico/multicore.h"

#if PICO_ON_DEVICE
// #include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "vga.h"
#else
#include <SDL2/SDL.h>
#endif

/* Atari and platform includes */
#include "mos6507/mos6507.h"
#include "atari/Atari-TIA.h"
#include "atari/Atari-cart.h"
#include "mos6532/mos6532.h"

// #define PRINT_STATE 1

/* Game cart data */
#include "cartridges/PaletteDemo.h"
#define CARTRIDGE PaletteDemo_bin

//#include "tia-tests/space.h"
//#define CARTRIDGE Space_Invaders_bin


#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 192

#if PICO_ON_DEVICE
const uint LED_PIN = 25;
static const sVmode *vmode = NULL;
struct semaphore vga_start_semaphore;
static uint32_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];
#else
SDL_Window *window;
SDL_Surface *window_surface;
SDL_TLSID cpu_core_ids;
static uint32_t screen[(SCREEN_WIDTH * SCREEN_HEIGHT) * 4];

void upscale(uint32_t *src, uint32_t *dest, int src_width, int src_height, int dest_width, int dest_height) {
    // Calculate scaling factors
    int w_scale = dest_width / src_width;
    int h_scale = dest_height / src_height;

    // Perform upscale operation
    for (int src_y = 0; src_y < src_height; src_y++) {
        for (int src_x = 0; src_x < src_width; src_x++) {
            for (int y_scale = 0; y_scale < h_scale; y_scale++) {
                for (int x_scale = 0; x_scale < w_scale; x_scale++) {
                    int dest_x = src_x * w_scale + x_scale;
                    int dest_y = src_y * h_scale + y_scale;
                    dest[dest_x + dest_y * dest_width] = src[src_x + src_y * src_width];
                }
            }
        }
    }
}
#endif

#if PICO_ON_DEVICE
/* Renderer loop on Pico's second core */
void __time_critical_func(render_loop)() {
    printf("Video on Core#%i running...\n", get_core_num());

    sem_acquire_blocking(&vga_start_semaphore);
    VgaInit(vmode, 640, 480);

    for (;;) {
        VgaLineBuf *linebuf = get_vga_line();
        uint32_t *buf = (uint32_t *)&(linebuf->line);
        uint32_t y = linebuf->row;

        if (y > 24 && y < SCREEN_HEIGHT + 24) {
            memcpy(buf, &screen[(y - 24) * 160], 160 * 4);
        } else {
            memset(buf, 0, 160*4);
        }

    }

    __builtin_unreachable();
}
#endif

void __time_critical_func(main_loop)() {
    uint32_t vsync = 0;
    uint32_t vblank = 0;
    uint32_t line_count = 0;
    int i, clock_count = 0;

    printf("Emulator on Core#%i running...\n", get_core_num());

    for (;;) {
#if !PICO_ON_DEVICE
        SDL_Event event;
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            if (window != NULL) {
                SDL_DestroyWindow(window);
            }

            SDL_Quit();
            return;
        }

        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            int pressed = event.type == SDL_KEYDOWN ? 1 : 0;
            if (event.key.keysym.sym == SDLK_UP) {
                mos6532_write(SWCHA, pressed ? 0b11101111 : 0b11111111);
            } else if (event.key.keysym.sym == SDLK_DOWN) {
                mos6532_write(SWCHA, pressed ? 0b11011111 : 0b11111111);
            } else if (event.key.keysym.sym == SDLK_LEFT) {
                mos6532_write(SWCHA, pressed ? 0b10111111 : 0b11111111);
            } else if (event.key.keysym.sym == SDLK_RIGHT) {
                mos6532_write(SWCHA, pressed ? 0b01111111 : 0b11111111);
            } else if (event.key.keysym.sym == SDLK_F1) {
                mos6532_write(SWCHB, pressed ? 0b00001110 : 0b00001111);    
            } else if (event.key.keysym.sym == SDLK_F2) {        
                mos6532_write(SWCHB, pressed ? 0b00001101 : 0b00001111);    
            } else if (event.key.keysym.sym == SDLK_F3) {
                // only toggle
                if (!pressed) {
                    uint8_t state;
                    mos6532_read(SWCHB, &state);
                    mos6532_write(SWCHB, state ^ (1 << 3));    
                }
                           } else if (event.key.keysym.sym == SDLK_F4) {
                // only toggle
                if (!pressed) {
                    uint8_t state;
                    mos6532_read(SWCHB, &state);
                    mos6532_write(SWCHB, state ^ (1 << 6));    
                }
            } else if (event.key.keysym.sym == SDLK_F5) {
                // only toggle
                if (!pressed) {
                    uint8_t state;
                    mos6532_read(SWCHB, &state);
                    mos6532_write(SWCHB, state ^ (1 << 7));    
                }
            } else if (event.key.keysym.sym == SDLK_SPACE) {
                TIA_joy1_state(pressed);
            }

        }
#endif

        // int c = getchar();
        // if (c == 27) {
        //     SDL_Quit();
        //     return;
        // }
        // printf("STEP \r\n");

        for (i = 0; i < TIA_COLOUR_CLOCK_TOTAL; i++) {
            clock_count = TIA_clock_tick();
            if (!TIA_get_WSYNC() && !((clock_count + 1) % 3))
            {
                mos6532_clock_tick();
                if (mos6507_clock_tick())
                {
                    return;
                }
            }
        }

        if (vsync && !TIA_get_VSYNC()) {
#if !PICO_ON_DEVICE
            upscale(screen, window_surface->pixels, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH * 4, SCREEN_HEIGHT * 2);
            SDL_UpdateWindowSurface(window);
#endif

            line_count = 0;
            vblank = TIA_VERTICAL_BLANK_LINES;
        }

        vsync = TIA_get_VSYNC();

        if (!vsync && !vblank && (line_count < TIA_VERTICAL_PICTURE_LINES)) {
#if !PICO_ON_DEVICE
            memcpy(&screen[(line_count * SCREEN_WIDTH)], tia_line_buffer, TIA_COLOUR_CLOCK_VISIBLE * 4);
#else
            memcpy(&screen[(line_count * SCREEN_WIDTH)], tia_raw_buffer, TIA_COLOUR_CLOCK_VISIBLE * 4);
#endif

            TIA_reset_buffer();
            line_count++;
        }

        if (vblank) {
            vblank--;
        }
    }
    __builtin_unreachable();
}

/******************************************************************************
 * Main code entry point
 *****************************************************************************/

int main()
{

#if PICO_ON_DEVICE
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    sleep_ms(50);
    set_sys_clock_khz(280 * 1000, true);
    sleep_ms(50);
    // stdio_init_all();
    sleep_ms(50);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    vmode = Video(DEV_VGA, RES_HVGA);
    // sleep_ms(5000);
    sem_init(&vga_start_semaphore, 0, 1);
    multicore_launch_core1(render_loop);
    sem_release(&vga_start_semaphore);

    gpio_put(LED_PIN, 1);
#else
    cpu_core_ids = SDL_TLSCreate();

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Atari 2600",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH * 4, SCREEN_HEIGHT * 2,
                              SDL_WINDOW_SHOWN);

    window_surface = SDL_GetWindowSurface(window);
#endif

    /* Setup and reset all the emulated
     * hardware: memory, CPU, TIA etc ...
     */
    opcode_populate_ISA_table();
    mos6532_init();
    TIA_init();

    /* Emulation is ready to start so load cartridge and reset CPU */
    cartridge_load(CARTRIDGE);
    mos6507_reset();

    main_loop();
}