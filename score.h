#ifndef SCORE_H
#define SCORE_H

// Return the stored best score or 0 if no file exists yet
int score_load_best(void);

// Store score as the new best only if it beats the old one
void score_save_best(int score);

#endif