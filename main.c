/*
 * Based on pspdev/psp/sdk/samples/gu/lines by Jesper Svennevid
 */

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <pspgu.h>

#include "callbacks.h"
#include "vram.h"

#define VERS 1
#define REVS 0

PSP_MODULE_INFO("waggers", PSP_MODULE_USER, VERS, REVS);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf

static unsigned int __attribute__((aligned(16))) list[262144];

typedef struct Point
{
  float x,y;
} Point;

struct Vertex
{
  float x,y,z;
};

typedef struct Scene
{
  Point player;
  Point waggers[10];
  int scroll;
  int apa;
} Scene;

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

unsigned int colors[8] =
  {
    0xffff0000,
    0xffff00ff,
    0xff0000ff,
    0xff00ffff,
    0xff00ff00,
    0xffffff00,
    0xffffffff,
    0xff00ffff
  };

#define NUM_VERTICES 4

int main(int argc, char* argv[])
{

  Scene scene;

  void* fbp0 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
  void* fbp1 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
  void* zbp = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_4444);

  int player_size = 25;

  scene.waggers[0].x = 200;
  scene.waggers[0].y = 100;

  scene.waggers[1].x = 300;
  scene.waggers[1].y = 160;

  scene.waggers[2].x = 400;
  scene.waggers[2].y = 200;

  scene.waggers[3].x = 500;
  scene.waggers[3].y = 130;

  int wagger_move = 1;
  int wagger_size = 50;

  SceCtrlData button_input;

  pspDebugScreenInit();
  setupCallbacks();

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  // initialize lines

  srand(time(0));

  // setup GU

  sceGuInit();

  sceGuStart(GU_DIRECT,list);
  sceGuDrawBuffer(GU_PSM_8888,fbp0,BUF_WIDTH);
  sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,fbp1,BUF_WIDTH);
  sceGuDepthBuffer(zbp,BUF_WIDTH);
  sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
  sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
  sceGuDepthRange(65535,0);
  sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
  sceGuEnable(GU_SCISSOR_TEST);
  sceGuFinish();
  sceGuSync(0,0);

  sceDisplayWaitVblankStart();
  sceGuDisplay(GU_TRUE);

  // run sample

  scene.scroll = 0;
  scene.player.x = 50;
  scene.player.y = 100;

  while(running()) {

    sceCtrlPeekBufferPositive(&button_input, 1);

    sceGuStart(GU_DIRECT,list);

    // clear screen
    sceGuClearColor(0);
    sceGuClear(GU_COLOR_BUFFER_BIT);

    // color
    sceGuColor(colors[0]);

    // draw vertices
    struct Vertex* vertices = sceGuGetMemory(4 * sizeof(struct Vertex));

    // button_input.Lx is something between 0 and 255
    float x_diff = button_input.Lx - 128;
    float y_diff = button_input.Ly - 128;

    if(abs(x_diff) > 20) {
      scene.scroll += (x_diff / 33);
    }
    if(abs(y_diff) > 20) {
      scene.player.y += (y_diff / 33);
    }
    printf("lX: %.6f , lY %.6f", x_diff, y_diff);

    vertices[0].x = scene.player.x;
    vertices[0].y = scene.player.y;

    vertices[1].x = scene.player.x + player_size;
    vertices[1].y = scene.player.y + player_size;

    vertices[2].x = scene.player.x;
    vertices[2].y = scene.player.y + player_size;

    vertices[3].x = scene.player.x;
    vertices[3].y = scene.player.y;

    sceGuDrawArray(GU_LINE_STRIP,
                   GU_VERTEX_32BITF|GU_TRANSFORM_2D,
                   NUM_VERTICES,
                   0,
                   vertices);


    int i = 0;
    for(i = 0; i < 4; i ++) {
      int wagger_x = scene.waggers[i].x;
      int wagger_y = scene.waggers[i].y;
      // wagger
      struct Vertex* wagger_vertices =
        sceGuGetMemory(5 * sizeof(struct Vertex));

      if(wagger_y < 25) {
        wagger_move = 1;
      }

      if(wagger_y > 200) {
        wagger_move = -1;
      }

      // color
      sceGuColor(colors[2]);

      scene.waggers[i].y += wagger_move;

      int wagx = wagger_x - scene.scroll;

      wagger_vertices[0].x = wagx;
      wagger_vertices[0].y = wagger_y;

      wagger_vertices[1].x = wagx + wagger_size;
      wagger_vertices[1].y = wagger_y + wagger_size;

      wagger_vertices[2].x = wagx;
      wagger_vertices[2].y = wagger_y + wagger_size;

      wagger_vertices[3].x = wagx;
      wagger_vertices[3].y = wagger_y;

      sceGuDrawArray(GU_LINE_STRIP,
                     GU_VERTEX_32BITF|GU_TRANSFORM_2D,
                     NUM_VERTICES,
                     0,
                     wagger_vertices);
    }

    // wait for next frame
    sceGuFinish();
    sceGuSync(0,0);

    pspDebugScreenSetXY(0, 0);
    if(button_input.Buttons != 0) {
      if(button_input.Buttons & PSP_CTRL_CROSS) {
        printf("x");
      }
      if(button_input.Buttons & PSP_CTRL_CIRCLE) {
        printf("o");
      }
      if(button_input.Buttons & PSP_CTRL_SQUARE) {
        printf("fyrkant");
      }
      if(button_input.Buttons & PSP_CTRL_TRIANGLE) {
        printf("triangel");
      }
    }

    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
  }

  sceGuTerm();

  sceKernelExitGame();
  return 0;
}
