#include <windows.h>
#include <iostream>
#include <filesystem>
#include <objbase.h>

using CreateAppWindow = bool(*)();
using UpdateAppWindow = bool(*)();
using CleanupAppWindow = void(*)();

void openConsole() {
    if (!AllocConsole()) {
        return;
    }

    SetConsoleTitleW(L"vEngineLiteNative Console");

    FILE* stream;

    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
    freopen_s(&stream, "CONIN$", "r", stdin);

    SetConsoleOutputCP(CP_UTF8);

    std::ios::sync_with_stdio(true);

    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();

    std::cout << "vEngineLiteNative console initialized\n";
}

int main(int argc, char* argv[]) {
    if (argc < 1 || !argv[0]) {
        return 1;
    }

    const auto exePath = std::filesystem::absolute(argv[0]);
    const auto dllDir = exePath.parent_path() / L"dll";

    if (!SetDllDirectoryW(dllDir.c_str())) {
        std::cerr << "Failed to set DLL directory.\n";
        return 1;
    }

    const auto runtimePath = dllDir / L"vEngineRuntime.dll";

    const HMODULE runtime = LoadLibraryW(runtimePath.c_str());

    if (!runtime) {
        std::cerr << "Failed to load vEngineRuntime.dll.\n";
        return 1;
    }

    const auto createAppWindow = reinterpret_cast<CreateAppWindow>(
        GetProcAddress(runtime, "createAppWindow")
    );

    const auto updateAppWindow = reinterpret_cast<UpdateAppWindow>(
        GetProcAddress(runtime, "updateAppWindow")
    );

    const auto cleanupAppWindow = reinterpret_cast<CleanupAppWindow>(
        GetProcAddress(runtime, "cleanupAppWindow")
    );

    if (!createAppWindow || !updateAppWindow || !cleanupAppWindow) {
        FreeLibrary(runtime);
        std::cerr << "Failed to find vEngineRuntime exported functions.\n";
        return 1;
    }

    const HRESULT hr = CoInitializeEx(
        nullptr,
        COINIT_MULTITHREADED
    );
    if (FAILED(hr)) {
        std::cerr << "CoInitializeEx failed.\n";
        return -1;
    }

    if (!createAppWindow()) {
        FreeLibrary(runtime);
        return 1;
    }

    while (updateAppWindow()) {}

    cleanupAppWindow();

    FreeLibrary(runtime);

    return 0;
}