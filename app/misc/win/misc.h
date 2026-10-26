#ifndef VENGINE_MISC_CPP_H
#define VENGINE_MISC_CPP_H

#include <string>
#include <v8.h>
#include "env.h"
#include "bitmap/bitmap.h"
#include "../fun.hpp"

std::wstring jsStrToWStr(const v8::String::Utf8Value& utf8);
FileContent<std::string> loadFileAsString(const Env* env, const std::string &fileName);
FileContent<Bitmap> loadBitmap(const Env* env, const std::string& fileName);
void CenterChildWindow(HWND parent, HWND child);

#endif