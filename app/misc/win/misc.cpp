
#include "misc.h"
#include <string>
#include <windows.h>
#include <v8.h>

std::wstring jsStrToWStr(
    const v8::String::Utf8Value& utf8)
{

    if (!*utf8) {
        return L"";
    }

    const int len = MultiByteToWideChar(
        CP_UTF8, 0, *utf8, -1, nullptr, 0
    );

    if (len <= 0) {
        return L"";
    }

    std::wstring result(len, L'\0');

    MultiByteToWideChar(
        CP_UTF8, 0, *utf8, -1, result.data(), len
    );

    result.pop_back(); // Видаляємо кінцевий '\0'
    return result;
}