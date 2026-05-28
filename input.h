#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

typedef struct
{
	unsigned char *spiled_base;
	uint8_t red, green, blue;
	int red_delta, green_delta, blue_delta;
	int red_btn, green_btn, blue_btn;
	int red_btn_prev, green_btn_prev, blue_btn_prev;
} input_t;

// Read initial knob state
void input_init(input_t *in, unsigned char *spiled_base);
// Reading knob input
void input_read(input_t *in);

// Press of each knob
int input_red_pressed(const input_t *in);
int input_green_pressed(const input_t *in);
int input_blue_pressed(const input_t *in);

#endif
