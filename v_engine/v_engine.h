
#ifndef VENGINE_V_ENGINE_H
#define VENGINE_V_ENGINE_H

#include "../app/js/Js.h"
#include "../app/js_compilation_result/JsCompilationResult.h"

class VEngine {
private:
    char* exePath;
    JsCompilationResult compileScript(std::string fileName, std::string source);
public:
    VEngine(char* exePath);
    void initV8();
    JsCompilationResult initGlobalHandlers();
    std::string getV8Version();
    JsCompilationResult compileInlineScript(std::string fileName, std::string source);
    JsCompilationResult compileScriptFromUrl(std::string url);
    void updateFrame();
    void onTouchEvent(std::string eventName, float x, float y, int id);
    void onResize(int width, int height);
    void dispose();

private:
    Js* js;
};


#endif