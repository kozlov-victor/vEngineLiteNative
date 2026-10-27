
#include "misc.h"
#include "env.h"
#include <string>
#include <windows.h>
#include <v8.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "bitmap/bitmap.h"
#include "../fun.hpp"

namespace fs = std::filesystem;

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

static std::string _clearUrl(const std::string &fileName) {
    // Відкидаємо query-параметри
    const size_t queryPos = fileName.find('?');
    return fileName.substr(0, queryPos);
}

FileContent<std::string> loadFileAsString(const Env* env, const std::string &fileName) {
    const fs::path executablePath = fs::absolute(fs::path(env->exePath));
    const fs::path filePath = executablePath.parent_path() / _clearUrl(fileName);

    // Read the script.
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: "
                  << filePath.string() << '\n';
        return {false,"no file: " + fileName,""};
    }
    std::string source{
        std::istreambuf_iterator{file},
        std::istreambuf_iterator<char>{}
    };
    if (source.size() >
        static_cast<size_t>(std::numeric_limits<int>::max())) {
        std::cerr << "file is too large.\n";
        return {false,"file is too large: " + fileName,""};
    }
    return {true, "",source};
}

FileContent<Bitmap> loadBitmap(const Env *env, const std::string &fileName) {
    const fs::path executablePath = fs::absolute(fs::path(env->exePath));
    const fs::path filePath = executablePath.parent_path() / _clearUrl(fileName);
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: "
                  << filePath.string() << '\n';
        return {false,"no file: " + fileName,{}};
    }
    return BitmapCache::loadBitmap(filePath);
}

void centerChildWindow(HWND parent, HWND child)
{
    RECT parentRect;
    RECT childRect;

    GetClientRect(parent, &parentRect);
    GetWindowRect(child, &childRect);

    const int parentWidth = parentRect.right - parentRect.left;
    const int parentHeight = parentRect.bottom - parentRect.top;

    const int childWidth = childRect.right - childRect.left;
    const int childHeight = childRect.bottom - childRect.top;

    const int x = (parentWidth - childWidth) / 2;
    const int y = (parentHeight - childHeight) / 2;

    SetWindowPos(
        child,
        nullptr,
        x, y,
        childWidth, childHeight,
        SWP_NOZORDER | SWP_NOACTIVATE
    );
}

bool parseCssColor(const std::string& color, V_COLOR& result) {
    if (color.empty() || color[0] != '#')
        return false;

    const std::string hex = color.substr(1);

    auto hexValue = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };

    auto parsePair = [&](std::size_t i) -> int {
        const int hi = hexValue(hex[i]);
        const int lo = hexValue(hex[i + 1]);
        if (hi < 0 || lo < 0) return -1;
        return hi * 16 + lo;
    };

    if (hex.size() == 3 || hex.size() == 4)
    {
        const int r = hexValue(hex[0]);
        const int g = hexValue(hex[1]);
        const int b = hexValue(hex[2]);

        if (r < 0 || g < 0 || b < 0)
            return false;

        result = {
            static_cast<std::uint8_t>(r * 17),
            static_cast<std::uint8_t>(g * 17),
            static_cast<std::uint8_t>(b * 17)
        };

        return true;
    }

    if (hex.size() == 6 || hex.size() == 8)
    {
        const int r = parsePair(0);
        const int g = parsePair(2);
        const int b = parsePair(4);

        if (r < 0 || g < 0 || b < 0)
            return false;

        result = {
            static_cast<std::uint8_t>(r),
            static_cast<std::uint8_t>(g),
            static_cast<std::uint8_t>(b)
        };

        return true;
    }

    return false;
}
