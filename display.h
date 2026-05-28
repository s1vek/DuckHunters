#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define LCD_W 480
#define LCD_H 320

#define RGB(r, g, b) (((((r) & 0xF8)) << 8) | ((((g) & 0xFC)) << 3) | ((b) >> 3))

#define COLOR_BLACK RGB(0, 0, 0)
#define COLOR_WHITE RGB(255, 255, 255)
#define COLOR_RED RGB(0, 255, 0)
#define COLOR_GREEN RGB(40, 200, 60)
#define COLOR_BLUE RGB(60, 120, 220)
#define COLOR_YELLOW RGB(240, 220, 40)
#define COLOR_GRAY RGB(140, 140, 140)
#define COLOR_DARK RGB(40, 40, 60)

// Initialize the display
void display_init(unsigned char *parlcd_base);
// Fill with single color
void display_clear(uint16_t color);
// Draw filled rectangle
void display_rect(int x, int y, int w, int h, uint16_t color);
// Draw text
void display_text(int x, int y, const char *s, uint16_t color, int scale);
// Pixel width of text
int display_text_width(const char *s, int scale);
// Push to display
void display_flush(void);

#endif
