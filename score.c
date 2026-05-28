#define _POSIX_C_SOURCE 200112L

#include <stdio.h>

#include "score.h"

#define SCORE_FILE "duck_hunters_score.txt"

int score_load_best(void)
{
	FILE *f = fopen(SCORE_FILE, "r");
	if (f == NULL) return 0;

	int best = 0;
	if (fscanf(f, "%d", &best) != 1) best = 0;
	fclose(f);
	return best;
}

void score_save_best(int score)
{
	if (score <= score_load_best()) return;

	FILE *f = fopen(SCORE_FILE, "w");
	if (f == NULL) return;

	fprintf(f, "%d\n", score);
	fclose(f);
}