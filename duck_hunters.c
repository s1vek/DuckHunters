/*******************************************************************
 *  duck_hunters.c - entry point of the Duck Hunters game for the
 *                   MZ_APO board.
 *
 *  Author: Sivek 
 *  Course: APO
 *
 *  Architecture:
 *    main() - maps peripherals, initializes the display and inputs,
 *             then runs a scene dispatcher loop that switches
 *             between menu / game / howto scenes based on each
 *             scene's return value.
 *
 *    display.c - LCD + drawing 
 *    input.c   - knob reading and button edge detection
 *    menu.c    - main menu
 *    game.c    - gameplay logic
 *    control.c   - controls overview
 *******************************************************************/

#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "serialize_lock.h"

#include "display.h"
#include "input.h"
#include "menu.h"
#include "input.h"
#include "scenes.h"
#include "game.h"
#include "control.h"

/* Map peripheral physical addresses into the process address space. */
static int map_peripherals(unsigned char **lcd_out, unsigned char **spiled_out)
{
	unsigned char *lcd = map_phys_address(PARLCD_REG_BASE_PHYS,
	                                      PARLCD_REG_SIZE, 0);
	if (lcd == NULL) {
		fprintf(stderr, "Failed to map PARLCD\n");
		return -1;
	}

	unsigned char *spiled = map_phys_address(SPILED_REG_BASE_PHYS,
	                                         SPILED_REG_SIZE, 0);
	if (spiled == NULL) {
		fprintf(stderr, "Failed to map SPILED\n");
		return -1;
	}

	*lcd_out = lcd;
	*spiled_out = spiled;
	return 0;
}

/* Main scene dispatcher - runs until the user chooses QUIT. */
static void run_main_loop(input_t *in, unsigned char *spiled)
{
	scene_t scene = SCENE_MENU;

	while (scene != SCENE_QUIT) {
		switch (scene) {
		case SCENE_MENU:
			scene = menu_run(in);
			break;
		case SCENE_GAME:
			scene = game_run(in, spiled);
			break;
		case SCENE_CONTROL:
			scene = control_run(in);
			break;
		default:
			scene = SCENE_QUIT;
			break;
		}
	}
}

int main(void)
{
    if (serialize_lock(1) <= 0) {
        printf("Application already running, waiting...\n");
        serialize_lock(0);
    }

    unsigned char *lcd_base = map_phys_address(PARLCD_REG_BASE_PHYS,
                                               PARLCD_REG_SIZE, 0);
    unsigned char *spiled_base = map_phys_address(SPILED_REG_BASE_PHYS,
                                                  SPILED_REG_SIZE, 0);
    if (lcd_base == NULL || spiled_base == NULL) {
        fprintf(stderr, "Failed to map peripherals\n");
        serialize_unlock();
        return 1;
    }

    display_init(lcd_base);

    input_t input;
    input_init(&input, spiled_base);

    run_main_loop(&input, spiled_base);

    display_clear(COLOR_BLACK);
    display_flush();
    serialize_unlock();
    return 0;
}