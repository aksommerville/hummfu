/* score.h
 */
 
#ifndef SCORE_H
#define SCORE_H

#define SCORE_LENGTH 6 /* You need so many digits to hold an encoded score. */

struct score {
  // Total across the entire session:
  double time; // scene
  int deathc; // hero
  // Counted only for completed levels:
  int killc; // hero
  int breakc; // hero
};

/* Represent (score) as a 6-digit decimal integer for display and storage.
 * We're not going to persist the broken-out score, just the digested number.
 * Output length is always SCORE_LENGTH (6), but we follow the usual convention for consistency.
 * If the buffer is large enough, we never fail to produce a valid representation, even of invalid scores.
 */
int score_rate(char *dst,int dsta,const struct score *score);

/* So you don't have to know how many digits.
 */
int score_is_zero(const char *score);

int score_is_valid(const char *score);

void score_force_valid(char *score);

#endif
