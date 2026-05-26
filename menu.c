#include <stdint.h>
#include <unistd.h>

#include "menu.h"
#include "display.h"

#define MENU_ITEMS 3
static const char *menu_labels[MENU_ITEMS] = {
	"START",
	"CONTROL",
	"EXIT"
};
static const scene_t menu_targets[MENU_ITEMS] = {
	SCENE_GAME,
	SCENE_CONTROL,
	SCENE_QUIT
};

static int update_selection(int selected, int knob_delta)
{
	const int deadband = 3;
	if (knob_delta > deadband) {
		selected++;
	} else if (knob_delta < -deadband) {
		selected--;
	}
	if (selected < 0) selected = MENU_ITEMS - 1;
	if (selected >= MENU_ITEMS) selected = 0;
	return selected;
}

static void draw_title(void)
{
	const char *title = "DUCK HUNTERS";
	int scale = 4;
	int w = display_text_width(title, scale);
	display_text((LCD_W - w) / 2, 40, title, COLOR_WHITE, scale);
}

static void draw_items(int selected)
{
	int scale = 2;
	int line_h = 50;
	int start_y = 150;

	for (int i = 0; i < MENU_ITEMS; i++) {
		int y = start_y + i * line_h;
		int w = display_text_width(menu_labels[i], scale);
		int x = (LCD_W - w) / 2;
		uint16_t color = (i == selected) ? COLOR_YELLOW : COLOR_WHITE;

		if (i == selected) {
			display_rect(x - 16, y - 6, w + 32, 16 * scale + 12,
			             COLOR_DARK);
		}
		display_text(x, y, menu_labels[i], color, scale);
	}
}

scene_t menu_run(input_t *in)
{
	int selected = 0;

	while (1) {
		input_read(in);
		selected = update_selection(selected, in->red_delta);

		if (input_green_pressed(in)) {
			return menu_targets[selected];
		}

		display_clear(COLOR_RED);
		draw_title();
		draw_items(selected);
		display_flush();

		usleep(20 * 1000);
	}
}
