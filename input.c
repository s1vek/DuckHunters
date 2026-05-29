#include <stdint.h>

#include "input.h"
#include "mzapo_regs.h"

// Read the 32-bit knob 
static uint32_t read_reg(unsigned char *base)
{
	return *(volatile uint32_t *)(base + SPILED_REG_KNOBS_8BIT_o);
}

static int signed_delta(uint8_t curr, uint8_t prev)
{
	return (int8_t)(curr - prev);
}

void input_init(input_t *in, unsigned char *spiled_base)
{
	in->spiled_base = spiled_base;
	uint32_t r = read_reg(spiled_base);
	in->blue  = (r >> 0)  & 0xFF;
	in->green = (r >> 8)  & 0xFF;
	in->red   = (r >> 16) & 0xFF;
	in->red_delta = in->green_delta = in->blue_delta = 0;
	in->red_btn   = (r >> 26) & 1;
	in->green_btn = (r >> 25) & 1;
	in->blue_btn  = (r >> 24) & 1;
	in->red_btn_prev   = in->red_btn;
	in->green_btn_prev = in->green_btn;
	in->blue_btn_prev  = in->blue_btn;
}

void input_read(input_t *in)
{
	uint32_t r = read_reg(in->spiled_base);
	uint8_t b = (r >> 0)  & 0xFF;
	uint8_t g = (r >> 8)  & 0xFF;
	uint8_t rd = (r >> 16) & 0xFF;
	in->blue_delta  = signed_delta(b,  in->blue);
	in->green_delta = signed_delta(g,  in->green);
	in->red_delta   = signed_delta(rd, in->red);
	in->blue = b; in->green = g; in->red = rd;
	in->red_btn_prev   = in->red_btn;
	in->green_btn_prev = in->green_btn;
	in->blue_btn_prev  = in->blue_btn;
	in->red_btn   = (r >> 26) & 1;
	in->green_btn = (r >> 25) & 1;
	in->blue_btn  = (r >> 24) & 1;
}

int input_red_pressed(const input_t *in)
{
	return in->red_btn && !in->red_btn_prev;
}

int input_green_pressed(const input_t *in)
{
	return in->green_btn && !in->green_btn_prev;
}

int input_blue_pressed(const input_t *in)
{
	return in->blue_btn && !in->blue_btn_prev;
}
