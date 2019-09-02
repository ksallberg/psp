#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>

#include "callback.h"

#define VERS 1
#define REVS 0

PSP_MODULE_INFO("Kristians spel", PSP_MODULE_USER, VERS, REVS);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_MAX();

#define printf pspDebugScreenPrintf

int main(void)
{
  pspDebugScreenInit();
  setupExitCallback();

  while(isRunning()) {
    pspDebugScreenSetXY(20, 20);
    printf("Hello World! \n \n Test!!!");
    sceDisplayWaitVblankStart();
  }
  sceKernelExitGame();
  return 0;
}
