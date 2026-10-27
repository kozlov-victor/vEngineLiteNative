
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>

#include <chrono>
#include <iostream>

#include "app/v_engine/v_engine.h"
#include "app/misc/win/env.h"
#include "app/misc/win/misc.h"
#include "app/misc/win/keyboard/keyboard.h"
#include <objbase.h>

using Clock = std::chrono::steady_clock;

static constexpr wchar_t WINDOW_CLASS[] = L"vEngineLiteNativeWindow";
static constexpr wchar_t WINDOW_TITLE[] = L"vEngineLite Native";

VEngine* vEngine;
HWND window;
HWND canvas;
Env env{};

static LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {
        case WM_CLOSE: {
            if (!(GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_CHILD)) {
                DestroyWindow(hwnd);
            }
            return 0;
        }
        case WM_DESTROY: {
            if (!(GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_CHILD)) {
                PostQuitMessage(0);
            }
            return 0;
        }
        case WM_SIZE: {
            if (!canvas || !vEngine) return 0; // too early
            if (hwnd!=window) return 0; // this is child window
            RECT clientRect{};
            GetClientRect(hwnd, &clientRect);
            const int width = clientRect.right - clientRect.left;
            const int height = clientRect.bottom - clientRect.top;
            vEngine->onResize(width, height);
            CenterChildWindow(hwnd, canvas);
            return 0;
        }
        case WM_ERASEBKGND: {
            if (hwnd != canvas) {
                RECT rect{};
                GetClientRect(hwnd, &rect);
                FillRect(
                    reinterpret_cast<HDC>(wParam),
                    &rect,
                    GetSysColorBrush(COLOR_WINDOW)
                );
            }
            return 0;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            if (!vEngine) return 0;
            const std::string_view code = getKeyboardCode(lParam);
            vEngine->onKeyUpDown(code,true);
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            if (!vEngine) return 0;
            const std::string_view code = getKeyboardCode(lParam);
            vEngine->onKeyUpDown(code,false);
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

static HWND CreateAppWindow(HINSTANCE instance, int width, int height)
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

    RECT rect{0, 0, width, height};
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

static HWND CreateCanvasWindow(
    HWND parent,
    const int x,
    const int y,
    const int width,
    const int height)
{
    canvas = CreateWindowExW(
        0,
        WINDOW_CLASS,
        nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        x,
        y,
        width,
        height,
        parent,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    if (!canvas) {
        return nullptr;
    }

    return canvas;
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

    HRESULT hr = CoInitializeEx(
        nullptr,
        COINIT_MULTITHREADED
    );
    if (FAILED(hr)) {
        std::cerr << "CoInitializeEx failed.\n";
        return -1;
    }

    const int initialWidth = 1280;
    const int initialHeight = 720;

    HINSTANCE instance = GetModuleHandleW(nullptr);
    window = CreateAppWindow(instance, initialWidth, initialHeight);

    if (!window) {
        std::cerr << "CreateAppWindow failed.\n";
        return 1;
    }

    HWND canvasWindow = CreateCanvasWindow(
        window,
        0,
        0,
        300,
        300
    );

    if (!canvasWindow) {
        DestroyWindow(window);
        UnregisterClassW(WINDOW_CLASS, instance);
        return 1;
    }
    CenterChildWindow(window, canvas);

    // Initialize ANGLE / EGL.
    EGLDisplay display = CreateEglDisplay();

    if (display == EGL_NO_DISPLAY) {
        std::cerr << "Failed to get EGL display. EGL error: 0x"
                  << std::hex << eglGetError() << '\n';
        DestroyWindow(canvasWindow);
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
        DestroyWindow(canvasWindow);
        DestroyWindow(window);
        UnregisterClassW(WINDOW_CLASS, instance);
        return 1;
    }

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        std::cerr << "eglBindAPI failed. EGL error: 0x"
                  << std::hex << eglGetError() << '\n';
        eglTerminate(display);
        DestroyWindow(canvasWindow);
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
        EGL_DEPTH_SIZE, 24,
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
        DestroyWindow(canvasWindow);
        DestroyWindow(window);
        UnregisterClassW(WINDOW_CLASS, instance);
        return 1;
    }

    EGLSurface surface = eglCreateWindowSurface(
        display,
        config,
        canvasWindow,
        nullptr
    );

    if (surface == EGL_NO_SURFACE) {
        std::cerr << "eglCreateWindowSurface failed. EGL error: 0x"
                  << std::hex << eglGetError() << '\n';
        eglTerminate(display);
        DestroyWindow(canvasWindow);
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
        DestroyWindow(canvasWindow);
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
        DestroyWindow(canvasWindow);
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

    env.window = window;
    env.canvas = canvas;
    env.exePath = argv[0];
    vEngine = new VEngine(&env);
    JsCompilationResult result = vEngine->initV8(initialWidth, initialHeight);
    if (!result.success) {
        Logger::error(result.error);
        vEngine->dispose();
        delete vEngine;
        return 1;
    }

    auto [success, error, content] = loadFileAsString(&env, "index.js");
    if (!success) {
        Logger::error(result.error);
        vEngine->dispose();
        delete vEngine;
        return 1;
    };
    result = vEngine->compileScript("index.js", content);
    if (!result.success) {
        Logger::error(result.error);
        vEngine->dispose();
        delete vEngine;
        return 1;
    }


    while (true) {
        MSG message{};

        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        const auto now = Clock::now();

        // Wait until the next scheduled frame.
        if (now < nextFrame) {
            Sleep(1);
            continue;
        }

        RECT clientRect{};
        GetClientRect(canvasWindow, &clientRect);

        const int width = clientRect.right - clientRect.left;
        const int height = clientRect.bottom - clientRect.top;

        if (width > 0 && height > 0) {

            // Execute JavaScript once per frame.
            vEngine->updateFrame();

            if (!eglSwapBuffers(display, surface)) {
                std::cerr << "eglSwapBuffers failed. EGL error: 0x"
                          << std::hex << eglGetError() << '\n';
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
    vEngine->dispose();
    delete vEngine;

    eglMakeCurrent(
        display,
        EGL_NO_SURFACE,
        EGL_NO_SURFACE,
        EGL_NO_CONTEXT
    );

    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);

    if (IsWindow(canvasWindow)) {
        DestroyWindow(canvasWindow);
    }

    if (IsWindow(window)) {
        DestroyWindow(window);
    }

    UnregisterClassW(WINDOW_CLASS, instance);

    return 0;
}
