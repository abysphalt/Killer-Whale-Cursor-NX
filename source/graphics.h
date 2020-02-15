#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <switch.h>
#include "shader.h"

bool initEgl(NWindow* win);
void SwapBuffers();
void deinitEgl();

#endif
