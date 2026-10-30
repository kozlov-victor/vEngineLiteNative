

#include <iostream>
#include <objbase.h>

#include "app-main/app-main.h"

int main(int argc, char* argv[]) {
    if (argc < 1 || !argv[0]) {
        std::cerr << "Executable path is unavailable.\n";
        return 1;
    }

    HRESULT hr = CoInitializeEx(
        nullptr,
        COINIT_MULTITHREADED
    );
    if (FAILED(hr)) {
        std::cerr << "CoInitializeEx failed.\n";
        return -1;
    }

    createAppWindow(argv);

    while (true) {
        if (!updateAppWindow()) break;
    }

    cleanupAppWindow();

    return 0;
}
