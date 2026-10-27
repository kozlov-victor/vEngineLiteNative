

#ifndef VENGINE_ENV_H
#define VENGINE_ENV_H
#include "EGL/egl.h"

struct V_COLOR {
    int r = 0;
    int g = 0;
    int b = 0;
};

struct Env {
    HWND window;
    HWND canvas;
    char* exePath;
    V_COLOR background;
};

#endif