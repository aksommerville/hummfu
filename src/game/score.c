#include "game/hummfu.h"

#define TIME_WORST 120.000 /* Maximum session duration, for the time bonus. */
#define TIME_BEST   60.000 /* Session time to get the maximum time bonus. This is hard to full-clear, but totally possible. And so nice and round. */
#define TIME_BONUS_MAX  500000.0 /* Time bonus, if you reach TIME_BEST. The other end is zero. */
#define KILL_BONUS_MAX  250000.0 /* Extra points for killing every monster. */
#define BREAK_BONUS_MAX 250000.0 /* Extra points for breaking every box. */
#define DEATH_PENALTY    20000.0 /* Lose so much for each death. 50 deaths and you can't have a score at all. */

/* Rate and represent score.
 */
 
int score_rate(char *dst,int dsta,const struct score *score) {
  if (!dst) dsta=0;
  double points=0.0;
  
  // If the world contains no monsters or boxes, bonus is undefined. Call it zero by fiat.
  if (g.killc_max) points+=(score->killc*KILL_BONUS_MAX)/g.killc_max;
  if (g.breakc_max) points+=(score->breakc*BREAK_BONUS_MAX)/g.breakc_max;
  
  // Time scales continuously but clamps on both edges:
  if (score->time>=TIME_WORST) ;
  else if (score->time<=TIME_BEST) points+=TIME_BONUS_MAX;
  else points+=((TIME_WORST-score->time)*TIME_BONUS_MAX)/(TIME_WORST-TIME_BEST);
  
  // Lose points for each death:
  points-=score->deathc*DEATH_PENALTY;
  
  // And finally it's an integer no more than 999999 and no less than 1.
  int n;
  if (points>=999999.0) n=999999;
  else if (points<=1.0) n=1;
  else n=(int)points;
  
  // One more thing, in case the formula is imperfect: A saturated score must not be possible if any of the quantum fields is imperfect.
  if (n==999999) {
    if (
      score->deathc||
      (score->killc<g.killc_max)||
      (score->breakc<g.breakc_max)
    ) n=999998;
  // And just to be consistent, check it the other way too: If each constituent is perfect, the outcome must be too. This includes time.
  } else {
    if (
      !score->deathc&&
      (score->killc>=g.killc_max)&&
      (score->breakc>=g.breakc_max)&&
      (score->time<=TIME_BEST)
    ) n=999999;
  }
  
  if (dsta>=6) {
    dst[0]='0'+(n/100000)%10;
    dst[1]='0'+(n/ 10000)%10;
    dst[2]='0'+(n/  1000)%10;
    dst[3]='0'+(n/   100)%10;
    dst[4]='0'+(n/    10)%10;
    dst[5]='0'+(n       )%10;
    if (dsta>6) dst[6]=0;
  }
  return 6;
}

/* Trivial ops against encoded scores.
 */

int score_is_zero(const char *score) {
  char zero[SCORE_LENGTH];
  memset(zero,'0',SCORE_LENGTH);
  return memcmp(score,zero,SCORE_LENGTH)?0:1;
}

int score_is_perfect(const char *score) {
  char nines[SCORE_LENGTH];
  memset(nines,'9',SCORE_LENGTH);
  return memcmp(score,nines,SCORE_LENGTH)?0:1;
}

int score_is_valid(const char *score) {
  int i=SCORE_LENGTH;
  for (;i-->0;score++) {
    if (*score<'0') return 0;
    if (*score>'9') return 0;
  }
  return 1;
}

void score_force_valid(char *score) {
  if (score_is_valid(score)) return;
  memset(score,'0',SCORE_LENGTH);
}

int score_rate_time(double time) {
  if (time<=TIME_BEST) return 1;
  if (time>=TIME_WORST) return -1;
  return 0;
}

double score_best_time() {
  return TIME_BEST;
}
