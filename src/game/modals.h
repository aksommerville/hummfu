/* modals.h
 */
 
#ifndef MODALS_H
#define MODALS_H

struct gameover {
  int active;
  int msg_texid,msg_w,msg_h;
  int new_hiscore;
};

void gameover_update(struct gameover *gameover,double elapsed,int input,int pvinput);
void gameover_render(struct gameover *gameover);
int gameover_begin(struct gameover *gameover);

#endif
