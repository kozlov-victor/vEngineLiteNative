
#ifndef VENGINE_V_ENGINE_H
#define VENGINE_V_ENGINE_H

#include "../js/Js.h"
#include "../js_compilation_result/JsCompilationResult.h"

class VEngine {
private:
    void* env;
    Js* js;
    JsCompilationResult initGlobalHandlers();
    JsCompilationResult initBootstrap();
public:
    explicit VEngine(void* env);
    JsCompilationResult initV8(int screenWidth, int screenHeight);
    std::string getV8Version();
    JsCompilationResult compileScript(const std::string& fileName, const std::string& source) const;
    void updateFrame() const;
    void onTouchEvent(std::string_view eventName, float x, float y, int id) const;
    void onKeyUpDown(std::string_view keyCode, bool down) const;
    void onResize(int width, int height) const;
    void dispose();
};


#endif