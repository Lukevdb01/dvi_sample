#include <stdio.h>
#include "pico/stdlib.h"
#include "dvi.h"
#include "dvi_serialiser.h"
#include "dvi_timing.h"
#include "common_dvi_pin_configs.h"

// Simple 320x240 RGB332 color bar pattern for DVI output
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240

uint8_t framebuffer[FRAME_HEIGHT][FRAME_WIDTH];

void fill_color_bars() {
    for (int y = 0; y < FRAME_HEIGHT; ++y) {
        for (int x = 0; x < FRAME_WIDTH; ++x) {
            if (x < FRAME_WIDTH / 3) framebuffer[y][x] = 0xE0; // Red
            else if (x < 2 * FRAME_WIDTH / 3) framebuffer[y][x] = 0x1C; // Green
            else framebuffer[y][x] = 0x03; // Blue
        }
    }
}

int main() {
    stdio_init_all();
    fill_color_bars();

    struct dvi_inst dvi0 = {0};
    dvi0.timing = &dvi_timing_640x480p_60hz;
    dvi0.ser_cfg = waveshare_rp2040_pizero;
    dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());
    dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
    dvi_start(&dvi0);

    while (true) {
        // For 8bpp framebuffer, use dvi_scanbuf_main_8bpp
        dvi_scanbuf_main_8bpp(&dvi0);
    }
}
