#ifndef VENGINELITE_APP_MAIN_H
#define VENGINELITE_APP_MAIN_H

#ifdef VENGINE_RUNTIME_EXPORTS
    #define VENGINE_API __declspec(dllexport)
#else
    #define VENGINE_API __declspec(dllimport)
#endif

extern "C" {
    VENGINE_API bool createAppWindow(char* argv[]);
    VENGINE_API bool updateAppWindow();
    VENGINE_API void cleanupAppWindow();
}

#endif