#include <windows.h>
#include <iostream>
#include <filesystem>
#include <objbase.h>

using CreateAppWindow = bool(*)(char*[]);
using UpdateAppWindow = bool(*)();
using CleanupAppWindow = void(*)();

int main(int argc, char* argv[])
{
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

    HMODULE runtime = LoadLibraryW(runtimePath.c_str());

    if (!runtime) {
        std::cerr << "Failed to load vEngineRuntime.dll.\n";
        return 1;
    }

    auto createAppWindow = reinterpret_cast<CreateAppWindow>(
        GetProcAddress(runtime, "createAppWindow")
    );

    auto updateAppWindow = reinterpret_cast<UpdateAppWindow>(
        GetProcAddress(runtime, "updateAppWindow")
    );

    auto cleanupAppWindow = reinterpret_cast<CleanupAppWindow>(
        GetProcAddress(runtime, "cleanupAppWindow")
    );

    if (!createAppWindow || !updateAppWindow || !cleanupAppWindow) {
        FreeLibrary(runtime);
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

    if (!createAppWindow(argv)) {
        FreeLibrary(runtime);
        return 1;
    }

    while (updateAppWindow()) {}

    cleanupAppWindow();

    FreeLibrary(runtime);

    return 0;
}