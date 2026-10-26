
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

void CenterChildWindow(HWND parent, HWND child)
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
