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

#include "font.c"

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
  int wagger_len;
  int scroll;
  int goal;
} Scene;

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

static int fontwidthtab[128] = {
  10, 10, 10, 10,
  10, 10, 10, 10,
  10, 10, 10, 10,
  10, 10, 10, 10,

  10, 10, 10, 10,
  10, 10, 10, 10,
  10, 10, 10, 10,
  10, 10, 10, 10,

  10,  6,  8, 10, //   ! " #
  10, 10, 10,  6, // $ % & '
  10, 10, 10, 10, // ( ) * +
  6, 10,  6, 10, // , - . /

  10, 10, 10, 10, // 0 1 2 3
  10, 10, 10, 10, // 6 5 8 7
  10, 10,  6,  6, // 10 9 : ;
  10, 10, 10, 10, // < = > ?

  16, 10, 10, 10, // @ A B C
  10, 10, 10, 10, // D E F G
  10,  6,  8, 10, // H I J K
  8, 10, 10, 10, // L M N O

  10, 10, 10, 10, // P Q R S
  10, 10, 10, 12, // T U V W
  10, 10, 10, 10, // X Y Z [
  10, 10,  8, 10, // \ ] ^ _

  6,  8,  8,  8, // ` a b c
  8,  8,  6,  8, // d e f g
  8,  6,  6,  8, // h i j k
  6, 10,  8,  8, // l m n o

  8,  8,  8,  8, // p q r s
  8,  8,  8, 12, // t u v w
  8,  8,  8, 10, // x y z {
  8, 10,  8, 12  // | } ~
};

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

#define NUM_VERTICES 5


void drawString(const char* text, int x, int y, unsigned int color, int fw) {
  int len = (int)strlen(text);
  if(!len) {
    return;
  }

  typedef struct {
    float s, t;
    unsigned int c;
    float x, y, z;
  } VERT;

  VERT* v = sceGuGetMemory(sizeof(VERT) * 2 * len);

  int i;
  for(i = 0; i < len; i++) {
    unsigned char c = (unsigned char)text[i];
    if(c < 32) {
      c = 0;
    } else if(c >= 128) {
      c = 0;
    }

    int tx = (c & 0x0F) << 4;
    int ty = (c & 0xF0);

    VERT* v0 = &v[i*2+0];
    VERT* v1 = &v[i*2+1];

    v0->s = (float)(tx + (fw ? ((16 - fw) >> 1) :
                          ((16 - fontwidthtab[c]) >> 1)));
    v0->t = (float)(ty);
    v0->c = color;
    v0->x = (float)(x);
    v0->y = (float)(y);
    v0->z = 0.0f;

    v1->s = (float)(tx + 16 - (fw ? ((16 - fw) >> 1) :
                               ((16 - fontwidthtab[c]) >> 1)));
    v1->t = (float)(ty + 16);
    v1->c = color;
    v1->x = (float)(x + (fw ? fw : fontwidthtab[c]));
    v1->y = (float)(y + 16);
    v1->z = 0.0f;

    x += (fw ? fw : fontwidthtab[c]);
  }

  sceGumDrawArray(GU_SPRITES,
                  GU_TEXTURE_32BITF | GU_COLOR_8888 |
                  GU_VERTEX_32BITF | GU_TRANSFORM_2D,
                  len * 2, 0, v
                  );
}

int collides(Scene *scene, int index) {
  int wagger_x = scene->waggers[index].x;
  int wagger_y = scene->waggers[index].y;
  int wagger_size = 50;
  int player_size = 25;
  int player_right_x = scene->player.x + scene->scroll + player_size;
  int player_left_x = scene->player.x + scene->scroll;
  if(player_right_x >= wagger_x &&
     player_left_x <= wagger_x + wagger_size) {
    int player_top_y = scene->player.y;
    int player_bottom_y = scene->player.y + player_size;
    if(player_top_y <= wagger_y + wagger_size &&
       player_bottom_y >= wagger_y) {
      return 1;
    }
  }
  return 0;
}

void draw_wagger(struct Vertex *vertices, Scene *scene, int index) {
  int wagger_x = scene->waggers[index].x;
  int wagger_y = scene->waggers[index].y;
  int wagger_size = 50;

  int does_collide = collides(scene, index);
  if(does_collide) {
    sceGuColor(colors[4]);
  } else {
    sceGuColor(colors[2]);
  }

  int wagx = wagger_x - scene->scroll;

  vertices[0].x = wagx;
  vertices[0].y = wagger_y;

  vertices[1].x = wagx + wagger_size;
  vertices[1].y = wagger_y;

  vertices[2].x = wagx + wagger_size;
  vertices[2].y = wagger_y + wagger_size;

  vertices[3].x = wagx;
  vertices[3].y = wagger_y + wagger_size;

  vertices[4].x = wagx;
  vertices[4].y = wagger_y;

  sceGuDrawArray(GU_LINE_STRIP,
                 GU_VERTEX_32BITF|GU_TRANSFORM_2D,
                 NUM_VERTICES,
                 0,
                 vertices);
}

void draw_player(struct Vertex *vertices, Scene *scene) {

  int player_size = 25;

  vertices[0].x = scene->player.x;
  vertices[0].y = scene->player.y;

  vertices[1].x = scene->player.x + player_size;
  vertices[1].y = scene->player.y;

  vertices[2].x = scene->player.x + player_size;
  vertices[2].y = scene->player.y + player_size;

  vertices[3].x = scene->player.x;
  vertices[3].y = scene->player.y + player_size;

  vertices[4].x = scene->player.x;
  vertices[4].y = scene->player.y;

  // color
  sceGuColor(colors[3]);

  sceGuDrawArray(GU_LINE_STRIP,
                 GU_VERTEX_32BITF|GU_TRANSFORM_2D,
                 NUM_VERTICES,
                 0,
                 vertices);
}

draw_goal(struct Vertex *vertices, Scene *scene) {

  vertices[0].x = scene->goal - scene->scroll;
  vertices[0].y = 0;

  vertices[1].x = scene->goal - scene->scroll;
  vertices[1].y = 500;

  int num_vertices = 2;

  // color
  sceGuColor(colors[5]);

  sceGuDrawArray(GU_LINE_STRIP,
                 GU_VERTEX_32BITF|GU_TRANSFORM_2D,
                 num_vertices,
                 0,
                 vertices);
}

scene_for_level(Scene *scene, int level) {
  switch(level) {
  case 1:
    scene->goal = 300;
    scene->wagger_len = 1;
    scene->waggers[0].x = 200;
    scene->waggers[0].y = 100;
    scene->scroll = 0;
    scene->player.x = 50;
    scene->player.y = 100;
    break;

  case 2:
    scene->goal = 400;
    scene->wagger_len = 2;
    scene->waggers[0].x = 200;
    scene->waggers[0].y = 100;
    scene->waggers[1].x = 300;
    scene->waggers[1].y = 160;
    scene->scroll = 0;
    scene->player.x = 50;
    scene->player.y = 100;
    break;

  case 3:
    scene->goal = 600;
    scene->wagger_len = 4;
    scene->waggers[0].x = 200;
    scene->waggers[0].y = 100;
    scene->waggers[1].x = 300;
    scene->waggers[1].y = 160;
    scene->waggers[2].x = 400;
    scene->waggers[2].y = 200;
    scene->waggers[3].x = 500;
    scene->waggers[3].y = 130;
    scene->scroll = 0;
    scene->player.x = 50;
    scene->player.y = 100;
    break;


  default:
    scene->goal = 0;
  }
}

int main(int argc, char* argv[])
{
  int level = 1;

  int in_level = 1;
  int player_size = 25;

  int health = 100;

  Scene scene;

  void* fbp0 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
  void* fbp1 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
  void* zbp = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_4444);

  scene_for_level(&scene, level);

  int wagger_move = 1;

  SceCtrlData button_input;

  pspDebugScreenInit();
  setupCallbacks();

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  // initialize lines

  srand(time(0));

  // setup GU
  sceGuInit();
  sceGuStart(GU_DIRECT, list);
  sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_WIDTH);
  sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
  sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
  sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
  sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
  sceGuDepthRange(0xc350,0x2710);
  sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
  sceGuEnable(GU_SCISSOR_TEST);
  sceGuDisable(GU_DEPTH_TEST);
  sceGuShadeModel(GU_SMOOTH);
  sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
  sceGuEnable(GU_TEXTURE_2D);
  sceGuTexMode(GU_PSM_8888, 0, 0, 0);
  sceGuTexImage(0, 256, 128, 256, font);
  sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
  sceGuTexEnvColor(0x0);
  sceGuTexOffset(0.0f, 0.0f);
  sceGuTexScale(1.0f / 256.0f, 1.0f / 128.0f);
  sceGuTexWrap(GU_REPEAT, GU_REPEAT);
  sceGuTexFilter(GU_NEAREST, GU_NEAREST);
  sceGuFinish();
  sceGuSync(0,0);
  sceGuDisplay(GU_TRUE);

  // game loop
  while(running()) {

    sceCtrlPeekBufferPositive(&button_input, 1);
    sceGuStart(GU_DIRECT,list);

    // clear screen
    sceGuClearColor(0);
    sceGuClear(GU_COLOR_BUFFER_BIT);

    if(health > 0) {
      if(in_level) {
        sceGuDisable(GU_BLEND); // needed for text but makes all else invisible

        // draw vertices
        struct Vertex* player_vertices =
          sceGuGetMemory(NUM_VERTICES * sizeof(struct Vertex));

        draw_player(player_vertices, &scene);

        // button_input.Lx is something between 0 and 255
        float x_diff = button_input.Lx - 128;
        float y_diff = button_input.Ly - 128;

        if(abs(x_diff) > 20) {
          scene.scroll += (x_diff / 33);
        }
        // Scroll bounds
        if(scene.scroll < 0) {
          scene.scroll = 0;
        }

        if(abs(y_diff) > 20) {
          scene.player.y += (y_diff / 33);
        }

        // Player bounds
        if(scene.player.y < 0) {
          scene.player.y = 0;
        } else if(scene.player.y + player_size > SCR_HEIGHT) {
          scene.player.y = SCR_HEIGHT - player_size;
        }
        printf("lX: %.6f , lY %.6f", x_diff, y_diff);

        int i;
        for(i = 0; i < scene.wagger_len; i ++) {
          // wagger
          struct Vertex* wagger_vertices =
            sceGuGetMemory(5 * sizeof(struct Vertex));

          int does_collide = collides(&scene, i);
          if(does_collide) {
            health --;
          }

          int wagger_x = scene.waggers[i].x;
          int wagger_y = scene.waggers[i].y;

          if(wagger_y < 25) {
            wagger_move = 1;
          }

          if(wagger_y > 200) {
            wagger_move = -1;
          }

          scene.waggers[i].y += wagger_move;
          draw_wagger(wagger_vertices, &scene, i);
        }

        // goal vertices
        struct Vertex* goal_vertices =
          sceGuGetMemory(2 * sizeof(struct Vertex));

        draw_goal(goal_vertices, &scene);

        if(scene.scroll + scene.player.x + player_size >= scene.goal) {
          in_level = 0;
        }

        sceGuEnable(GU_BLEND); // needed for text
        char health_str[3];
        snprintf(health_str, 6, "%d", health);
        drawString(health_str, 10, 10, 0xFFC0FFEE, 0);

        // If not in level
      } else {
        sceGuEnable(GU_BLEND); // needed for text
        if(level == 3) {
          drawString("Du klarade spelet!", 50, 128, 0xFFFFFFFF, 0);
        } else {
          char level_str[1];
          snprintf(level_str, 2, "%d", level);
          drawString("Du klarade banan  !", 50, 50, 0xFFC0FFEE, 0);
          drawString(level_str, 185, 50, 0xFFC0FFEE, 0);
          drawString("WOW - tryck X for att fortsatta", 50, 128, 0xFFFFFFFF, 0);
          drawString("WOW", 50, 144, 0x7FFFFFFF, 0);
          drawString("WOW", 50, 160, 0x18FFFFFF, 0);
        }
      }
    } else {
      in_level = 0;
      sceGuEnable(GU_BLEND); // needed for text
      drawString("GAME OVER!", 50, 50, 0xFFFF0000, 0);
      drawString("tryck X for att spela igen", 50, 70, 0xFFFFFFFF, 0);
    }

    // wait for next frame
    sceGuFinish();
    sceGuSync(0,0);

    pspDebugScreenSetXY(0, 0);
    if(button_input.Buttons != 0) {
      if(button_input.Buttons & PSP_CTRL_CROSS) {
        if(in_level == 0 && level < 3) {
          in_level = 1;
          level ++;
          scene_for_level(&scene, level);
        } else if (in_level == 0 && health <= 0) {
          in_level = 1;
          level = 1;
          health = 100;
          scene_for_level(&scene, level);
        }
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
