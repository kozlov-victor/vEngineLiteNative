

#ifndef VENGINE_ENV_H
#define VENGINE_ENV_H
#include "EGL/egl.h"

struct Env {
    HWND window;
    HWND canvas;
    char* exePath;
};

#endif