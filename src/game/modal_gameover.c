#include "hummfu.h"

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
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x800000ff);
  //TODO
}

/* Begin.
 */
 
int gameover_begin(struct gameover *gameover) {
  g.scene.active=0;
  g.hello.active=0;
  gameover->active=1;
  //TODO
  return 0;
}
