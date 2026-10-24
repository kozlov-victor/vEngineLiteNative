
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>

#include "JsRuntime/JsRuntime.h"

#include <chrono>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;

static constexpr wchar_t WINDOW_CLASS[] = L"vEngineLiteNativeWindow";
static constexpr wchar_t WINDOW_TITLE[] = L"vEngineLite Native";

static LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

static HWND CreateAppWindow(HINSTANCE instance)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // Число 32512 — це системний ідентифікатор стандартного курсора-стрілки Windows (OCR_NORMAL / IDC_ARROW).
    wc.lpszClassName = WINDOW_CLASS;

    if (!RegisterClassExW(&wc)) {
        std::cerr << "RegisterClassExW failed: "
                  << GetLastError() << '\n';
        return nullptr;
    }

    RECT rect{0, 0, 1280, 720};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExW(
        0,
        WINDOW_CLASS,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        instance,
        nullptr
    );

    if (!hwnd) {
        std::cerr << "CreateWindowExW failed: "
                  << GetLastError() << '\n';
        UnregisterClassW(WINDOW_CLASS, instance);
        return nullptr;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;
}

static EGLDisplay CreateEglDisplay()
{
    auto getPlatformDisplay =
        reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT")
        );

    if (!getPlatformDisplay) {
        std::cerr << "eglGetPlatformDisplayEXT unavailable.\n";
        return EGL_NO_DISPLAY;
    }

    const EGLint displayAttributes[] = {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_NONE
    };

    return getPlatformDisplay(
        EGL_PLATFORM_ANGLE_ANGLE,
        EGL_DEFAULT_DISPLAY,
        displayAttributes
    );
}

int main(int argc, char* argv[])
{
    if (argc < 1 || !argv[0]) {
        std::cerr << "Executable path is unavailable.\n";
        return 1;
    }

    HINSTANCE instance = GetModuleHandleW(nullptr);
    HWND window = CreateAppWindow(instance);

    if (!window) {
        return 1;
    }

    // Initialize ANGLE / EGL.
    EGLDisplay display = CreateEglDisplay();

    if (display == EGL_NO_DISPLAY) {
        std::cerr << "Failed to get EGL display. EGL error: 0x"
                  << std::hex << eglGetError() << '\n';
        DestroyWindow(window);
        UnregisterClassW(WINDOW_CLASS, instance);
        return 1;
    }

    EGLint major = 0;
    EGLint minor = 0;

    if (!eglInitialize(display, &major, &minor)) {
        std::cerr << "eglInitialize failed. EGL error: 0x"
                  << std::hex << eglGetError() << '\n';
        eglTerminate(display);
        DestroyWindow(window);
        UnregisterClassW(WINDOW_CLASS, instance);
        return 1;
    }

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        std::cerr << "eglBindAPI failed. EGL error: 0x"
                  << std::hex << eglGetError() << '\n';
        eglTerminate(display);
        DestroyWindow(window);
        UnregisterClassW(WINDOW_CLASS, instance);
        return 1;
    }

    const EGLint configAttributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    EGLConfig config{};
    EGLint configCount = 0;

    if (!eglChooseConfig(
            display,
            configAttributes,
            &config,
            1,
            &configCount) ||
        configCount == 0) {
        std::cerr << "eglChooseConfig failed. EGL error: 0x"
                  << std::hex << eglGetError() << '\n';
        eglTerminate(display);
        DestroyWindow(window);
        UnregisterClassW(WINDOW_CLASS, instance);
        return 1;
    }

    EGLSurface surface = eglCreateWindowSurface(
        display,
        config,
        window,
        nullptr
    );

    if (surface == EGL_NO_SURFACE) {
        std::cerr << "eglCreateWindowSurface failed. EGL error: 0x"
                  << std::hex << eglGetError() << '\n';
        eglTerminate(display);
        DestroyWindow(window);
        UnregisterClassW(WINDOW_CLASS, instance);
        return 1;
    }

    const EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    EGLContext context = eglCreateContext(
        display,
        config,
        EGL_NO_CONTEXT,
        contextAttributes
    );

    if (context == EGL_NO_CONTEXT) {
        std::cerr << "eglCreateContext failed. EGL error: 0x"
                  << std::hex << eglGetError() << '\n';
        eglDestroySurface(display, surface);
        eglTerminate(display);
        DestroyWindow(window);
        UnregisterClassW(WINDOW_CLASS, instance);
        return 1;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        std::cerr << "eglMakeCurrent failed. EGL error: 0x"
                  << std::hex << eglGetError() << '\n';
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
        DestroyWindow(window);
        UnregisterClassW(WINDOW_CLASS, instance);
        return 1;
    }

    // Our own scheduler controls the target rate.
    eglSwapInterval(display, 0);

    std::cout << "EGL version: " << major << '.' << minor << '\n';
    std::cout << "GL_VERSION: "
              << reinterpret_cast<const char*>(glGetString(GL_VERSION))
              << '\n';
    std::cout << "GL_RENDERER: "
              << reinterpret_cast<const char*>(glGetString(GL_RENDERER))
              << '\n';

    constexpr double targetFps = 60.0;

    const auto frameDuration =
        std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(1.0 / targetFps)
        );

    const auto startTime = Clock::now();
    auto nextFrame = startTime;


    // Initialize JavaScript before entering the main loop.
    const fs::path executablePath =
            fs::absolute(fs::path(argv[0]));

    const fs::path scriptPath =
        executablePath.parent_path() / "index.js";

    JsRuntime jsRuntime;
    if (!jsRuntime.Initialize(scriptPath, executablePath)) {
        MessageBoxW(
            nullptr,
            L"Помилка JavaScript. Дивись vengine.log біля EXE.",
            L"vEngineLite Native",
            MB_OK | MB_ICONERROR
        );
        return 1;
    }

    bool running = true;

    while (running) {
        MSG message{};

        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                running = false;
                break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        if (!running) {
            break;
        }

        const auto now = Clock::now();

        // Wait until the next scheduled frame.
        if (now < nextFrame) {
            Sleep(1);
            continue;
        }

        const double timestampMs =
            std::chrono::duration<double, std::milli>(
                now - startTime
            ).count();

        // Execute JavaScript once per frame.
        if (!jsRuntime.Tick(timestampMs)) {
            break;
        }

        RECT clientRect{};
        GetClientRect(window, &clientRect);

        const int width = clientRect.right - clientRect.left;
        const int height = clientRect.bottom - clientRect.top;

        if (width > 0 && height > 0) {
            glViewport(0, 0, width, height);
            glClearColor(0.05f, 0.08f, 0.9f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            if (!eglSwapBuffers(display, surface)) {
                std::cerr << "eglSwapBuffers failed. EGL error: 0x"
                          << std::hex << eglGetError() << '\n';
                running = false;
                break;
            }
        }

        // Maintain a 60 FPS schedule without catch-up bursts.
        nextFrame += frameDuration;

        const auto afterFrame = Clock::now();

        while (nextFrame <= afterFrame) {
            nextFrame += frameDuration;
        }
    }

    // Tear down JS while its owner is still alive.
    jsRuntime.Shutdown();

    eglMakeCurrent(
        display,
        EGL_NO_SURFACE,
        EGL_NO_SURFACE,
        EGL_NO_CONTEXT
    );

    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);

    if (IsWindow(window)) {
        DestroyWindow(window);
    }

    UnregisterClassW(WINDOW_CLASS, instance);

    return 0;
}
