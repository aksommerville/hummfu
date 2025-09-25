/* modals.h
 * Defines "hello" and "gameover".
 * These are both singletons on the global object.
 */
 
#ifndef MODALS_H
#define MODALS_H

struct hello {
  int active;
};

struct gameover {
  int active;
};

void hello_update(struct hello *hello,double elapsed,int input,int pvinput);
void hello_render(struct hello *hello);
int hello_begin(struct hello *hello);

void gameover_update(struct gameover *gameover,double elapsed,int input,int pvinput);
void gameover_render(struct gameover *gameover);
int gameover_begin(struct gameover *gameover);

#endif
