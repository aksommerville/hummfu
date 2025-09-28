/* modals.h
 */
 
#ifndef MODALS_H
#define MODALS_H

#define GAMEOVER_LABEL_LIMIT 3

struct gameover {
  int active;
  struct gameover_label {
    int texid;
    int x,y,w,h;
    int blink;
  } labelv[GAMEOVER_LABEL_LIMIT];
  int labelc;
  const void *strings; // strings:1
  int stringsc;
  char score[SCORE_LENGTH];
};

void gameover_update(struct gameover *gameover,double elapsed,int input,int pvinput);
void gameover_render(struct gameover *gameover);
int gameover_begin(struct gameover *gameover);

#endif
