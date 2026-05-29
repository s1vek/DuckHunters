#ifndef GAME_H
#define GAME_H

#include "input.h"
#include "scenes.h"
#include "display.h"

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
#define COLOR_DUCK    RGB(200, 60, 60)
#define COLOR_BEAK    RGB(255, 140, 0)

// A single duck
typedef struct {
	int active;
	int x, y;
	int vx;
} duck_t;

// Full game state passed between logic and rendering
typedef struct {
	int cx, cy;
	int score;
	int lives;
	int spawn_timer;
	int hit_flash, miss_flash;
	duck_t ducks[MAX_DUCKS];
} game_t;

// Run one full game
scene_t game_run(input_t *in, unsigned char *spiled_base);

#endif