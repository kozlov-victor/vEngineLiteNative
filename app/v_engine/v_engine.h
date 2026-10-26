
#ifndef VENGINE_V_ENGINE_H
#define VENGINE_V_ENGINE_H

#include "../js/Js.h"
#include "../misc/win/env.h"
#include "../js_compilation_result/JsCompilationResult.h"

class VEngine {
private:
    Env* env;
    JsCompilationResult compileScript(std::string fileName, std::string source);
public:
    explicit VEngine(Env* env);
    JsCompilationResult initV8(int screenWidth, int screenHeight);
    std::string getV8Version();
    JsCompilationResult compileInlineScript(std::string fileName, std::string source);
    JsCompilationResult compileScriptFromUrl(const std::string &url);
    void updateFrame() const;
    void onTouchEvent(std::string eventName, float x, float y, int id) const;
    void onResize(int width, int height) const;
    void dispose();

private:
    Js* js{};
    JsCompilationResult initGlobalHandlers();
    JsCompilationResult initBootstrap();
};


#endif