#define _POSIX_C_SOURCE 200112L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "game.h"
#include "ducks.h"
#include "render.h"
#include "input.h"
#include "display.h"
#include "mzapo_regs.h"
#include "pause.h"
#include "score.h"

// Write a 32-bit value
static void write_reg(unsigned char *base, int off, uint32_t v)
{
	*(volatile uint32_t *)(base + off) = v;
}

// RGB LED green on hit / red on miss; LED line shows lives
static void leds_update(unsigned char *spiled, const game_t *g)
{
	uint32_t rgb = 0;
	if (g->hit_flash > 0)  rgb = 0x0000FF00;
	if (g->miss_flash > 0) rgb = 0x00FF0000;

	uint32_t line = 0;
	for (int i = 0; i < g->lives && i < 3; i++)
		line |= 0x3FFu << (i * 11);

	write_reg(spiled, SPILED_REG_LED_RGB1_o, rgb);
	write_reg(spiled, SPILED_REG_LED_RGB2_o, rgb);
	write_reg(spiled, SPILED_REG_LED_LINE_o, line);
}

// Turn all LEDs off
static void leds_off(unsigned char *spiled)
{
	write_reg(spiled, SPILED_REG_LED_RGB1_o, 0);
	write_reg(spiled, SPILED_REG_LED_RGB2_o, 0);
	write_reg(spiled, SPILED_REG_LED_LINE_o, 0);
}

// Move crosshair from knob deltas, clamp to screen
static void move_crosshair(game_t *g, input_t *in)
{
	g->cx += in->red_delta  * KNOB_SPEED;
	g->cy += in->blue_delta * KNOB_SPEED;
	if (g->cx < 0)      g->cx = 0;
	if (g->cx >= LCD_W) g->cx = LCD_W - 1;
	if (g->cy < HUD_H)  g->cy = HUD_H;
	if (g->cy >= LCD_H) g->cy = LCD_H - 1;
}

// Game over screen, waits for a button press
static void show_game_over(input_t *in, int score)
{
	char score_buf[32], best_buf[32];
	int best = score_load_best();
	snprintf(score_buf, sizeof(score_buf), "SCORE: %d", score);
	snprintf(best_buf,  sizeof(best_buf),  "BEST: %d",  best);

	while (1) {
		input_read(in);
		if (input_green_pressed(in) || input_red_pressed(in)) return;

		display_clear(COLOR_WHITE);
		int tw = display_text_width("GAME OVER", 4);
		display_text((LCD_W - tw) / 2, 60, "GAME OVER", COLOR_BLACK, 4);
		int sw = display_text_width(score_buf, 3);
		display_text((LCD_W - sw) / 2, 150, score_buf, COLOR_BLACK, 3);
		int bw = display_text_width(best_buf, 3);
		display_text((LCD_W - bw) / 2, 200, best_buf, COLOR_BLACK, 3);
		const char *hint = "PRESS GREEN TO CONTINUE";
		int hw = display_text_width(hint, 1);
		display_text((LCD_W - hw) / 2, 270, hint, COLOR_BLACK, 1);
		display_flush();
	}
}

scene_t game_run(input_t *in, unsigned char *spiled)
{
	game_t g = {
		.cx = LCD_W / 2, .cy = LCD_H / 2,
		.lives = LIVES_START, .spawn_timer = 30,
	};
	srand((unsigned)time(NULL));

	while (g.lives > 0) {
		input_read(in);

		if (input_blue_pressed(in) && pause_run(in) == SCENE_MENU) {
			leds_off(spiled);
			return SCENE_MENU;
		}

		move_crosshair(&g, in);

		if (input_green_pressed(in)) {
			if (ducks_try_shoot(&g)) {
				g.score++;
				g.hit_flash = FLASH_FRAMES;
			} else {
				g.lives--;
				g.miss_flash = FLASH_FRAMES;
				if (g.lives < 0) g.lives = 0;
			}
		}

		ducks_try_spawn(&g);
		int missed = ducks_step(&g);
		if (missed > 0) {
			g.lives -= missed;
			g.miss_flash = FLASH_FRAMES;
			if (g.lives < 0) g.lives = 0;
		}

		render_scene(&g);
		display_flush();
		leds_update(spiled, &g);

		if (g.hit_flash  > 0) g.hit_flash--;
		if (g.miss_flash > 0) g.miss_flash--;
		usleep(FRAME_USLEEP);
	}

	leds_off(spiled);
	score_save_best(g.score);
	show_game_over(in, g.score);
	return SCENE_MENU;
}