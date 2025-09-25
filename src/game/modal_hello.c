#include "hummfu.h"

/* Update.
 */

void hello_update(struct hello *hello,double elapsed,int input,int pvinput) {
  //TODO
}

/* Render.
 */
 
void hello_render(struct hello *hello) {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x008000ff);
  //TODO
}

/* Begin.
 */
 
int hello_begin(struct hello *hello) {
  g.scene.active=0;
  g.gameover.active=0;
  hello->active=1;
  //TODO
  return 0;
}
