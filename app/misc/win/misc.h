#ifndef VENGINE_MISC_CPP_H
#define VENGINE_MISC_CPP_H

#include <string>
#include <v8.h>

std::wstring jsStrToWStr(const v8::String::Utf8Value& utf8);

#endif