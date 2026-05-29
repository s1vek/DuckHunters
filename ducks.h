#ifndef DUCKS_H
#define DUCKS_H

#include "game.h"

// Spawn if allowed + manage timer
void ducks_try_spawn(game_t *g);   
// Move ducks, return number that flew off
int  ducks_step(game_t *g);
// Return 1 if crosshair hit a duck      
int  ducks_try_shoot(game_t *g);

#endif