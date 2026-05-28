#define _POSIX_C_SOURCE 200112L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "game.h"
#include "display.h"
#include "input.h"
#include "mzapo_regs.h"
#include "pause.h"
#include "score.h"

#define MAX_DUCKS     12
#define DUCK_W        50
#define DUCK_H        26
#define CROSS_R       18
#define HUD_H         36
#define LIVES_START   3
#define KNOB_SPEED    4
#define FLASH_FRAMES  8
#define BASE_SPAWN    120   
#define MIN_SPAWN     30
#define BASE_VX       2
#define FRAME_USLEEP  15000

#define COLOR_SKY     RGB(120, 180, 230)
#define COLOR_DUCK    RGB(180, 120, 40)
#define COLOR_DUCK    RGB(200, 60, 60)
#define COLOR_BELLY   RGB(245, 225, 180)
#define COLOR_BEAK    RGB(255, 140, 0)

typedef struct {
	int active;
	int x, y;
	int vx;
} duck_t;

typedef struct {
	int cx, cy;
	int score;
	int lives;
	int frame;
	int spawn_timer;
	int hit_flash, miss_flash;
	duck_t ducks[MAX_DUCKS];
} game_t;


static void write_reg(unsigned char *base, int off, uint32_t v)
{
	*(volatile uint32_t *)(base + off) = v;
}

// Update rbg 
static void leds_update(unsigned char *spiled, const game_t *g)
{
	uint32_t rgb = 0;
	if (g->hit_flash > 0)  rgb = 0x0000FF00;  
	if (g->miss_flash > 0) rgb = 0x00FF0000;  

	uint32_t line = 0;
	for (int i = 0; i < g->lives && i < 3; i++) {
		line |= 0x3FFu << (i * 11);  
	}

	write_reg(spiled, SPILED_REG_LED_RGB1_o, rgb);
	write_reg(spiled, SPILED_REG_LED_RGB2_o, rgb);
	write_reg(spiled, SPILED_REG_LED_LINE_o, line);
}

// Spawn of ducks from each sides
static void duck_spawn(duck_t *d, int score)
{
	int level = score / 6;
	int vx = BASE_VX + level / 3; 
	int left = rand() & 1;
	d->active = 1;
	d->y = HUD_H + 20 + rand() % (LCD_H - HUD_H - DUCK_H - 80);
	d->x  = left ? -DUCK_W : LCD_W;
	d->vx = left ?  vx     : -vx;
}

// Count active ducks
static int count_active(const game_t *g)
{
	int n = 0;
	for (int i = 0; i < MAX_DUCKS; i++)
		if (g->ducks[i].active) n++;
	return n;
}

// Spawn of ducks if its allowed
static void try_spawn(game_t *g)
{
	if (g->spawn_timer > 0) { g->spawn_timer--; return; }

	int level = g->score / 6;
	int allowed = 2 + level;
	if (allowed > MAX_DUCKS) allowed = MAX_DUCKS;

	if (count_active(g) >= allowed) {
		g->spawn_timer = 10; 
		return;
	}

	for (int i = 0; i < MAX_DUCKS; i++) {
		if (!g->ducks[i].active) {
			duck_spawn(&g->ducks[i], g->score);
			break;
		}
	}
	int interval = BASE_SPAWN - level * 3;
	if (interval < MIN_SPAWN) interval = MIN_SPAWN;
	g->spawn_timer = interval;
}

// Moving of ducks
static int ducks_step(game_t *g)
{
	int missed = 0;
	for (int i = 0; i < MAX_DUCKS; i++) {
		duck_t *d = &g->ducks[i];
		if (!d->active) continue;
		d->x += d->vx;
		if ((d->vx > 0 && d->x > LCD_W) ||
		    (d->vx < 0 && d->x + DUCK_W < 0)) {
			d->active = 0;
			missed++;
		}
	}
	return missed;
}

// Check crosshair and duck, return 1 if hit
static int try_shoot(game_t *g)
{
	for (int i = 0; i < MAX_DUCKS; i++) {
		duck_t *d = &g->ducks[i];
		if (!d->active) continue;
		if (g->cx >= d->x && g->cx < d->x + DUCK_W &&
		    g->cy >= d->y && g->cy < d->y + DUCK_H) {
			d->active = 0;
			return 1;
		}
	}
	return 0;
}

// Draw ducks
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

// Draw crosshair
static void draw_crosshair(int x, int y, uint16_t color)
{
	display_rect(x - CROSS_R, y - 1, CROSS_R * 2, 2, color);
	display_rect(x - 1, y - CROSS_R, 2, CROSS_R * 2, color);
	display_rect(x - 3, y - 3, 6, 6, color);
}

// Draw HUD
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

// Draw full frame
static void draw_scene(const game_t *g)
{
	display_clear(COLOR_SKY);

	for (int i = 0; i < MAX_DUCKS; i++) draw_duck(&g->ducks[i]);

	uint16_t c = COLOR_BLACK;
	if (g->hit_flash > 0)  c = COLOR_GREEN;
	if (g->miss_flash > 0) c = COLOR_WHITE;
	draw_crosshair(g->cx, g->cy, c);

	draw_hud(g);
}

// Gameover screen
static void show_game_over(input_t *in, int score)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "SCORE: %d", score);

	while (1) {
		input_read(in);
		if (input_green_pressed(in) || input_red_pressed(in)) return;

		display_clear(COLOR_WHITE);
		int tw = display_text_width("GAME OVER", 4);
		display_text((LCD_W - tw) / 2, 80, "GAME OVER", COLOR_BLACK, 4);
		int sw = display_text_width(buf, 3);
		display_text((LCD_W - sw) / 2, 170, buf, COLOR_BLACK, 3);
		const char *hint = "PRESS GREEN TO CONTINUE";
		int hw = display_text_width(hint, 1);
		display_text((LCD_W - hw) / 2, 250, hint, COLOR_BLACK, 1);
		display_flush();
	}
}

// Main gameloop
scene_t game_run(input_t *in, unsigned char *spiled)
{
	game_t g = {
		.cx = LCD_W / 2, .cy = LCD_H / 2,
		.lives = LIVES_START, .spawn_timer = 30,
	};
	srand((unsigned)time(NULL));

	while (g.lives > 0) {
		input_read(in);

        if (input_blue_pressed(in)) {
	    if (pause_run(in) == SCENE_MENU) {
		write_reg(spiled, SPILED_REG_LED_RGB1_o, 0);
		write_reg(spiled, SPILED_REG_LED_RGB2_o, 0);
		write_reg(spiled, SPILED_REG_LED_LINE_o, 0);
		return SCENE_MENU;
	    }
        }   

		g.cx += in->red_delta  * KNOB_SPEED;
		g.cy += in->blue_delta * KNOB_SPEED;
		if (g.cx < 0)        g.cx = 0;
		if (g.cx >= LCD_W)   g.cx = LCD_W - 1;
		if (g.cy < HUD_H)    g.cy = HUD_H;
		if (g.cy >= LCD_H)   g.cy = LCD_H - 1;

		if (input_green_pressed(in)) {
			if (try_shoot(&g)) { g.score++; g.hit_flash = FLASH_FRAMES; }
			else               { g.miss_flash = FLASH_FRAMES; }
		}

		try_spawn(&g);
		int missed = ducks_step(&g);
		if (missed > 0) {
			g.lives -= missed;
			g.miss_flash = FLASH_FRAMES;
			if (g.lives < 0) g.lives = 0;
		}

		draw_scene(&g);
		display_flush();
		leds_update(spiled, &g);

		if (g.hit_flash  > 0) g.hit_flash--;
		if (g.miss_flash > 0) g.miss_flash--;
		g.frame++;
		usleep(FRAME_USLEEP);
	}

	write_reg(spiled, SPILED_REG_LED_RGB1_o, 0);
	write_reg(spiled, SPILED_REG_LED_RGB2_o, 0);
	write_reg(spiled, SPILED_REG_LED_LINE_o, 0);
	score_save_best(g.score);
	show_game_over(in, g.score);
	return SCENE_MENU;
}