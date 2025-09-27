#include "hummfu.h"

#define BG_COLOR 0x106620ff

/* Update.
 */

void gameover_update(struct gameover *gameover,double elapsed,int input,int pvinput) {
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) {
    scene_begin(&g.scene,1);
  }
  //TODO
}

/* Render.
 */
 
void gameover_render(struct gameover *gameover) {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,BG_COLOR);
  graf_set_input(&g.graf,gameover->msg_texid);
  graf_decal(&g.graf,(FBW>>1)-(gameover->msg_w>>1),(FBH>>1)-(gameover->msg_h>>1),0,0,gameover->msg_w,gameover->msg_h);
  //TODO
}

/* Begin.
 */
 
int gameover_begin(struct gameover *gameover) {
  g.scene.active=0;
  gameover->active=1;
  egg_play_song(RID_song_peace_returns,0,0);
  
  if (!gameover->msg_texid) {
    gameover->msg_texid=hummfu_load_label(&gameover->msg_w,&gameover->msg_h,6);
  }
  //TODO
  return 0;
}
