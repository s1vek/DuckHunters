/*
 * Shared enum for scene switching.
 *
 * Each scene has a *_run() function that runs until the user
 * triggers a switch, and returns the next scene as its result.
 */

#ifndef SCENES_H
#define SCENES_H

typedef enum {
    SCENE_MENU,
    SCENE_GAME,
    SCENE_CONTROL,
    SCENE_QUIT
} scene_t;

#endif