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
  
  fprintf(stderr,"%s time=%f deathc=%d killc=%d/%d breakc=%d/%d\n",__func__,g.score.time,g.score.deathc,g.score.killc,g.killc_max,g.score.breakc,g.breakc_max);
  
  //TODO generate piecemeal score labels
  
  /* Tabulating and storing the final score is our job.
   * Somebody else populates (g.score) and we clear it.
   */
  char score[SCORE_LENGTH];
  score_rate(score,sizeof(score),&g.score);
  fprintf(stderr,"score=%.6s\n",score);
  if (memcmp(score,g.hiscore,SCORE_LENGTH)>0) {
    fprintf(stderr,"...new hi score (prev=%.6s)\n",g.hiscore);
    gameover->new_hiscore=1;
    memcpy(g.hiscore,score,SCORE_LENGTH);
    egg_store_set("hiscore",7,score,SCORE_LENGTH);
    //TODO report "new high score"
  } else {
    fprintf(stderr,"...does not beat hiscore of %.6s\n",g.hiscore);
    gameover->new_hiscore=0;
    if (!score_is_zero(g.hiscore)) {
      //TODO report hiscore
    }
  }
  memset(&g.score,0,sizeof(g.score));
  
  if (!gameover->msg_texid) {
    gameover->msg_texid=hummfu_load_label(&gameover->msg_w,&gameover->msg_h,6);
  }
  //TODO
  return 0;
}
