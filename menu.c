#include <stdint.h>
#include <unistd.h>

#include "menu.h"
#include "display.h"

#define MENU_ITEMS 3
static const char *menu_labels[MENU_ITEMS] = { "START", "CONTROL", "EXIT" };
static const scene_t menu_targets[MENU_ITEMS] = {
	SCENE_GAME, SCENE_CONTROL, SCENE_QUIT
};

static int update_selection(int selected, int knob_delta)
{
	static int accum = 0;
	accum += knob_delta;
	if (accum >=  4) { selected++; accum -= 4; }
	if (accum <= -4) { selected--; accum += 4; }
	if (selected < 0) selected = MENU_ITEMS - 1;
	if (selected >= MENU_ITEMS) selected = 0;
	return selected;
}

static void draw_title(void)
{
	const char *title = "DUCK HUNTERS";
	int w = display_text_width(title, 4);
	display_text((LCD_W - w) / 2, 40, title, COLOR_BLACK, 4);
}

static void draw_items(int selected)
{
	int line_h = 50, start_y = 150;
	for (int i = 0; i < MENU_ITEMS; i++) {
		int y = start_y + i * line_h;
		int w = display_text_width(menu_labels[i], 2);
		int x = (LCD_W - w) / 2;
		if (i == selected)
			display_rect(x - 16, y - 6, w + 32, 44, COLOR_YELLOW);
		display_text(x, y, menu_labels[i], COLOR_BLACK, 2);
	}
}

scene_t menu_run(input_t *in)
{
	int selected = 0;
	while (1) {
		input_read(in);
		selected = update_selection(selected, in->red_delta);

		display_clear(COLOR_WHITE);
		draw_title();
		draw_items(selected);
		display_flush();

		if (input_red_pressed(in)) return menu_targets[selected];
	}
}