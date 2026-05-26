#include <stdint.h>

#include "display.h"
#include "mzapo_parlcd.h"
#include "font_types.h"

extern font_descriptor_t font_rom8x16;

static uint16_t fb[LCD_H * LCD_W];
static unsigned char *lcd_base;

void display_init(unsigned char *parlcd_base)
{
	lcd_base = parlcd_base;
	parlcd_hx8357_init(lcd_base);
	parlcd_write_cmd(lcd_base, 0x3A);
	parlcd_write_data(lcd_base, 0x55);

	display_clear(COLOR_BLACK);
	display_flush();
}

void display_clear(uint16_t color)
{
	for (int i = 0; i < LCD_W * LCD_H; i++) {
		fb[i] = color;
	}
}

void display_rect(int x, int y, int w, int h, uint16_t color)
{
	int x0 = x < 0 ? 0 : x;
	int y0 = y < 0 ? 0 : y;
	int x1 = x + w > LCD_W ? LCD_W : x + w;
	int y1 = y + h > LCD_H ? LCD_H : y + h;
	for (int yy = y0; yy < y1; yy++) {
		for (int xx = x0; xx < x1; xx++) {
			fb[yy * LCD_W + xx] = color;
		}
	}
}

static void draw_char(int x, int y, char c, uint16_t color, int scale)
{
	int idx = (unsigned char)c - font_rom8x16.firstchar;
	if (idx < 0 || idx >= font_rom8x16.size) return;
	const font_bits_t *bits = &font_rom8x16.bits[idx * font_rom8x16.height];
	for (int row = 0; row < (int)font_rom8x16.height; row++) {
		font_bits_t line = bits[row];
		for (int col = 0; col < font_rom8x16.maxwidth; col++) {
			if (line & (0x8000 >> col)) {
				display_rect(x + col * scale, y + row * scale,
				             scale, scale, color);
			}
		}
	}
}

void display_text(int x, int y, const char *s, uint16_t color, int scale)
{
	if (scale < 1) scale = 1;
	int step = font_rom8x16.maxwidth * scale;
	for (int i = 0; s[i] != 0; i++) {
		draw_char(x + i * step, y, s[i], color, scale);
	}
}

int display_text_width(const char *s, int scale)
{
	if (scale < 1) scale = 1;
	int n = 0;
	while (s[n] != 0) n++;
	return n * font_rom8x16.maxwidth * scale;
}

void display_flush(void)
{
	parlcd_write_cmd(lcd_base, 0x2A);
	parlcd_write_data(lcd_base, 0x0000);
	parlcd_write_data(lcd_base, (uint16_t)(LCD_W - 1));

	parlcd_write_cmd(lcd_base, 0x2B);
	parlcd_write_data(lcd_base, 0x0000);
	parlcd_write_data(lcd_base, (uint16_t)(LCD_H - 1));

	parlcd_write_cmd(lcd_base, 0x2C);
	for (int i = 0; i < LCD_W * LCD_H; i++) {
		parlcd_write_data(lcd_base, fb[i]);
	}
}
