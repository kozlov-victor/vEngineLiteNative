
#ifndef VENGINE_V_ENGINE_H
#define VENGINE_V_ENGINE_H

#include "../app/js/Js.h"
#include "../app/misc/win/env.h"
#include "../app/js_compilation_result/JsCompilationResult.h"

class VEngine {
private:
    Env* env;
    JsCompilationResult compileScript(std::string fileName, std::string source);
public:
    VEngine(Env* env);
    void initV8();
    JsCompilationResult initGlobalHandlers();
    JsCompilationResult initBootstrap();
    std::string getV8Version();
    JsCompilationResult compileInlineScript(std::string fileName, std::string source);
    JsCompilationResult compileScriptFromUrl(const std::string &url);
    void updateFrame();
    void onTouchEvent(std::string eventName, float x, float y, int id);
    void onResize(int width, int height, bool triggerResize);
    void dispose();

private:
    Js* js;
};


#endif