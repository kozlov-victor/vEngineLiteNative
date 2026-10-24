#include <SDL3/SDL.h>
#include <GLES3/gl3.h>

#include "JsRuntime/JsRuntime.h"

#include <chrono>
#include <filesystem>
#include <iostream>

int main(int argc, char* argv[])
{
    namespace fs = std::filesystem;
    using Clock = std::chrono::steady_clock;

    if (argc < 1 || !argv[0]) {
        std::cerr << "Executable path is unavailable.\n";
        return 1;
    }

    const fs::path executablePath =
        fs::absolute(fs::path(argv[0]));

    const fs::path scriptPath =
        executablePath.parent_path() / "index.js";

    // Initialize JavaScript before entering the main loop.
    JsRuntime jsRuntime;

    if (!jsRuntime.Initialize(scriptPath, executablePath)) {
        return 1;
    }

    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_ES
    );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow(
        "vEngineLite Native",
        1280,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    if (!glContext) {
        SDL_Log("SDL_GL_CreateContext failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Our own scheduler controls the target rate.
    SDL_GL_SetSwapInterval(0);

    SDL_Log("GL_VERSION: %s",
        reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    SDL_Log("GL_RENDERER: %s",
        reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    constexpr double targetFps = 60.0;

    const auto frameDuration =
        std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(1.0 / targetFps)
        );

    const auto startTime = Clock::now();
    auto nextFrame = startTime;

    bool running = true;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        if (!running) {
            break;
        }

        const auto now = Clock::now();

        // Wait until the next scheduled frame.
        if (now < nextFrame) {
            SDL_Delay(1);
            continue;
        }

        const double timestampMs =
            std::chrono::duration<double, std::milli>(
                now - startTime
            ).count();

        // Execute JavaScript once per frame.
        if (!jsRuntime.Tick(timestampMs)) {
            running = false;
            break;
        }

        int width = 0;
        int height = 0;

        SDL_GetWindowSizeInPixels(window, &width, &height);

        glViewport(0, 0, width, height);
        glClearColor(0.05f, 0.08f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);

        // Maintain a 60 FPS schedule without catch-up bursts.
        nextFrame += frameDuration;

        const auto afterFrame = Clock::now();

        while (nextFrame <= afterFrame) {
            nextFrame += frameDuration;
        }
    }

    // Tear down JS while its owner is still alive.
    jsRuntime.Shutdown();

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}