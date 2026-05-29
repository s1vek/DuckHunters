#include <stdlib.h>

#include "ducks.h"

// Spawn one duck from a random side; speed grows with score
static void spawn_one(duck_t *d, int score)
{
	int level = score / 6;
	int vx = BASE_VX + level / 3;
	int left = rand() & 1;
	d->active = 1;
	d->y = HUD_H + 20 + rand() % (LCD_H - HUD_H - DUCK_H - 80);
	d->x  = left ? -DUCK_W : LCD_W;
	d->vx = left ? vx : -vx;
}

// Count ducks currently on screen
static int count_active(const game_t *g)
{
	int n = 0;
	for (int i = 0; i < MAX_DUCKS; i++)
		if (g->ducks[i].active) n++;
	return n;
}

void ducks_try_spawn(game_t *g)
{
	if (g->spawn_timer > 0) { g->spawn_timer--; return; }

	int level = g->score / 6;
	int allowed = 2 + level;
	if (allowed > MAX_DUCKS) allowed = MAX_DUCKS;

	if (count_active(g) >= allowed) { g->spawn_timer = 10; return; }

	for (int i = 0; i < MAX_DUCKS; i++) {
		if (!g->ducks[i].active) {
			spawn_one(&g->ducks[i], g->score);
			break;
		}
	}
	int interval = BASE_SPAWN - level * 3;
	if (interval < MIN_SPAWN) interval = MIN_SPAWN;
	g->spawn_timer = interval;
}

int ducks_step(game_t *g)
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

int ducks_try_shoot(game_t *g)
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