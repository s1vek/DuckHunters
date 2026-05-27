#include "control.h"
#include "display.h"

static void draw_screen(void)
{
	display_clear(COLOR_WHITE);

	const char *title = "CONTROLS";
	int tw = display_text_width(title, 3);
	display_text((LCD_W - tw) / 2, 30, title, COLOR_BLACK, 3);

	int y = 110, gap = 36;
	display_text(40, y,           "RED KNOB - MOVE X/MENU",  COLOR_BLACK, 2);
	display_text(40, y + gap,     "BLUE KNOB - MOVE Y",         COLOR_BLACK, 2);
	display_text(40, y + 2 * gap, "GREEN BTN - SHOOT/CONFIRM",COLOR_BLACK, 2);

	const char *hint = "PRESS RED TO RETURN";
	int hw = display_text_width(hint, 2);
	display_text((LCD_W - hw) / 2, LCD_H - 50, hint, COLOR_BLACK, 2);
}

scene_t control_run(input_t *in)
{
	while (1) {
		input_read(in);
		if (input_red_pressed(in)) return SCENE_MENU;
		draw_screen();
		display_flush();
	}
}