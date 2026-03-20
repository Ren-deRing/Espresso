#include <stdint.h>
#include <stddef.h>
#include <limine.h>

struct limine_framebuffer *g_framebuffer;

void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= g_framebuffer->width || y >= g_framebuffer->height) {
        return;
    }

    uint32_t* fb_ptr = (uint32_t*)g_framebuffer->address;
    size_t index = (y * (g_framebuffer->pitch / 4)) + x;

    fb_ptr[index] = color;
}