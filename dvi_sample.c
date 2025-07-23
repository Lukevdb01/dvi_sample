#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "dvi.h"
#include "dvi_timing.h"
#include "common_dvi_pin_configs.h"
#include "dvi_serialiser.h"

#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240

static uint16_t framebuffer[FRAME_WIDTH * FRAME_HEIGHT];
static struct dvi_inst dvi0;

void fill_color_bars() {
    for (int y = 0; y < FRAME_HEIGHT; y++) {
        for (int x = 0; x < FRAME_WIDTH; x++) {
            int idx = y * FRAME_WIDTH + x;
            if (x < FRAME_WIDTH / 3) {
                framebuffer[idx] = 0xF800; // Red in RGB565
            } else if (x < 2 * FRAME_WIDTH / 3) {
                framebuffer[idx] = 0x07E0; // Green in RGB565
            } else {
                framebuffer[idx] = 0x001F; // Blue in RGB565
            }
        }
    }
}

void core1_main() {
    dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
    dvi_start(&dvi0);
    dvi_scanbuf_main_16bpp(&dvi0);
}

int main() {
    stdio_init_all();
    fill_color_bars();

    dvi0.timing = &dvi_timing_640x480p_60hz; // You can use dvi_timing_320x240 if available, but 640x480p_60hz works for centered bars
    dvi0.ser_cfg = waveshare_rp2040_pizero;
    dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

    multicore_launch_core1(core1_main);

    // Enqueue pointers to each scanline for the scanout core
    while (true) {
        for (int y = 0; y < FRAME_HEIGHT; ++y) {
            uintptr_t scanline_ptr = (uintptr_t)(&framebuffer[y * FRAME_WIDTH]);
            queue_add_blocking_u32(&dvi0.q_colour_valid, &scanline_ptr);
            while (queue_try_remove_u32(&dvi0.q_colour_free, &scanline_ptr)) {
                // drain free queue
            }
        }
    }
    return 0;
}