

#ifndef VENGINELITE_KEYBOARD_H
#define VENGINE_KEYBOARD_H

#include <Windows.h>
#include <string_view>

std::string_view getKeyboardCode(LPARAM lParam);

#endif