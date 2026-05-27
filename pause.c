#include "pause.h"
#include "display.h"

#define ITEMS 2
static const char  *labels[ITEMS]  = { "RESUME", "BACK TO MENU" };
static const scene_t targets[ITEMS] = { SCENE_GAME, SCENE_MENU };

static void draw_panel(int selected)
{
	int bw = 340, bh = 180;
	int bx = (LCD_W - bw) / 2, by = (LCD_H - bh) / 2;

	display_rect(bx, by, bw, bh, COLOR_WHITE);
	display_rect(bx, by, bw, 3, COLOR_BLACK);
	display_rect(bx, by + bh - 3, bw, 3, COLOR_BLACK);
	display_rect(bx, by, 3, bh, COLOR_BLACK);
	display_rect(bx + bw - 3, by, 3, bh, COLOR_BLACK);

	const char *title = "PAUSED";
	int tw = display_text_width(title, 3);
	display_text((LCD_W - tw) / 2, by + 18, title, COLOR_BLACK, 3);

	for (int i = 0; i < ITEMS; i++) {
		int iw = display_text_width(labels[i], 2);
		int ix = (LCD_W - iw) / 2;
		int iy = by + 88 + i * 38;
		if (i == selected)
			display_rect(ix - 12, iy - 4, iw + 24, 28, COLOR_YELLOW);
		display_text(ix, iy, labels[i], COLOR_BLACK, 2);
	}
}

scene_t pause_run(input_t *in)
{
	int selected = 0, accum = 0;
	while (1) {
		input_read(in);

		accum += in->red_delta;
		if (accum >=  4) { selected = (selected + 1) % ITEMS; accum -= 4; }
		if (accum <= -4) { selected = (selected + 1) % ITEMS; accum += 4; }

		if (input_green_pressed(in)) return targets[selected];
		if (input_blue_pressed(in))  return SCENE_GAME;

		draw_panel(selected);
		display_flush();
	}
}