#ifndef SCORE_H
#define SCORE_H

/*******************************************************************
 *  score.h - persistent best-score storage.
 *
 *  The best score is kept in a plain text file so it survives
 *  between runs of the game.
 *******************************************************************/

/* Return the stored best score, or 0 if no file exists yet. */
int score_load_best(void);

/* Store 'score' as the new best, but only if it beats the old one. */
void score_save_best(int score);

#endif /* SCORE_H */