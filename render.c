#include <stdio.h>

#include "render.h"
#include "display.h"

// Draw a duck body, belly stripe, head and beak
static void draw_duck(const duck_t *d)
{
	if (!d->active) return;

	display_rect(d->x, d->y, DUCK_W, DUCK_H, COLOR_DUCK);
	display_rect(d->x + 4, d->y + DUCK_H - 8, DUCK_W - 8, 4, COLOR_YELLOW);

	int hx = d->vx > 0 ? d->x + DUCK_W - 14 : d->x + 2;
	display_rect(hx, d->y - 10, 14, 14, COLOR_DUCK);

	int bx = d->vx > 0 ? hx + 16 : hx - 8;
	display_rect(bx, d->y - 4, 8, 5, COLOR_BEAK);
}

// Draw the crosshair as a cross with a centre square
static void draw_crosshair(int x, int y, uint16_t color)
{
	display_rect(x - CROSS_R, y - 1, CROSS_R * 2, 2, color);
	display_rect(x - 1, y - CROSS_R, 2, CROSS_R * 2, color);
	display_rect(x - 3, y - 3, 6, 6, color);
}

// Draw the top bar with score and lives
static void draw_hud(const game_t *g)
{
	char buf[32];
	display_rect(0, 0, LCD_W, HUD_H, COLOR_WHITE);

	snprintf(buf, sizeof(buf), "SCORE %d", g->score);
	display_text(10, 10, buf, COLOR_BLACK, 2);

	snprintf(buf, sizeof(buf), "LIVES %d", g->lives);
	int w = display_text_width(buf, 2);
	display_text(LCD_W - w - 10, 10, buf, COLOR_BLACK, 2);
}

void render_scene(const game_t *g)
{
	display_clear(COLOR_SKY);
	for (int i = 0; i < MAX_DUCKS; i++) draw_duck(&g->ducks[i]);
	draw_crosshair(g->cx, g->cy, COLOR_BLACK);
	draw_hud(g);
}