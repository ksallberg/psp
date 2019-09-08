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

PSP_MODULE_INFO("programmet", PSP_MODULE_USER, VERS, REVS);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf

static unsigned int __attribute__((aligned(16))) list[262144];

struct Vertex
{
  float x,y,z;
};

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

  void* fbp0 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
  void* fbp1 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
  void* zbp = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_4444);

  int x_pos = 100;
  int y_pos = 100;
  int player_size = 25;

  int wagger_x = 200;
  int wagger_y = 200;
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

    x_pos = button_input.Lx;
    y_pos = button_input.Ly;

    vertices[0].x = x_pos;
    vertices[0].y = y_pos;

    vertices[1].x = x_pos + player_size;
    vertices[1].y = y_pos + player_size;

    vertices[2].x = x_pos;
    vertices[2].y = y_pos + player_size;

    vertices[3].x = x_pos;
    vertices[3].y = y_pos;

    sceGuDrawArray(GU_LINE_STRIP,
                   GU_VERTEX_32BITF|GU_TRANSFORM_2D,
                   NUM_VERTICES,
                   0,
                   vertices);
    // wagger
    struct Vertex* wagger_vertices = sceGuGetMemory(4 * sizeof(struct Vertex));

    if(wagger_y < 25) {
      wagger_move = 1;
    }

    if(wagger_y > 200) {
      wagger_move = -1;
    }

    // color
    sceGuColor(colors[2]);

    wagger_y += wagger_move;

    wagger_vertices[0].x = wagger_x;
    wagger_vertices[0].y = wagger_y;

    wagger_vertices[1].x = wagger_x + wagger_size;
    wagger_vertices[1].y = wagger_y + wagger_size;

    wagger_vertices[2].x = wagger_x;
    wagger_vertices[2].y = wagger_y + wagger_size;

    wagger_vertices[3].x = wagger_x;
    wagger_vertices[3].y = wagger_y;

    sceGuDrawArray(GU_LINE_STRIP,
                   GU_VERTEX_32BITF|GU_TRANSFORM_2D,
                   NUM_VERTICES,
                   0,
                   wagger_vertices);

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
