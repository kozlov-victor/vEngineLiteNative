#include "v_engine.h"
#include <filesystem>
#include <iostream>
#include <v8-version-string.h>
#include "../app/misc/win/env.h"
#include "../app/bootstrap/bootstrap.hpp"

namespace fs = std::filesystem;

VEngine::VEngine(Env* env):env(env) {
}

void VEngine::initV8() {
    js = new Js();
    js->initV8();
    js->initGlobalObjects(env);
}

JsCompilationResult VEngine::initGlobalHandlers() {
    bool success = true;
    std::string result;
    if (!js->findFunctionByName("_nextTick",js->nextTickFunction)) {
        success = false;
        result = "no _nextTick function; bootstrap.h is wrong";
    }
    else if (!js->findFunctionByName("_triggerEvent", js->triggerEventFunction)) {
        success = false;
        result = "no _triggerEvent function; bootstrap.h is wrong";
    }
    return JsCompilationResult(success,result);
}

JsCompilationResult VEngine::initBootstrap() {
    return compileInlineScript("bootstrap.js", bootstrapJs);
}

std::string VEngine::getV8Version() {
    return V8_VERSION_STRING;
}

JsCompilationResult VEngine::compileScript(std::string fileName, std::string source) {
    return
        js->compileScript(fileName.c_str(),
            source.c_str()
        );
}

JsCompilationResult VEngine::compileInlineScript(std::string fileName, std::string source) {
    return compileScript(fileName, source);
}

JsCompilationResult VEngine::compileScriptFromUrl(std::string url) {
    const fs::path executablePath = fs::absolute(fs::path(env->exePath));
    const fs::path scriptPath = executablePath.parent_path() / url;

    // Read the script.
    std::ifstream file(scriptPath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open JavaScript file: "
                  << scriptPath.string() << '\n';
        return JsCompilationResult(false,"no file: " + url);
    }
    std::string source{
        std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}
    };
    if (source.size() >
        static_cast<size_t>(std::numeric_limits<int>::max())) {
        std::cerr << "JavaScript file is too large.\n";
        return JsCompilationResult(false,"JavaScript file is too large: " + url);
    }

    return compileScript(url, source);
}

void VEngine::updateFrame() {
    if (js == nullptr) {
        return;
    }
    js->callFunction(js->nextTickFunction);
}

void VEngine::onTouchEvent(std::string eventName, float x, float y, int id) {
    if (js == nullptr) {
        return;
    }
    js->onTouchEvent(eventName.c_str(),x,y,id);
}

void VEngine::onResize(int width, int height) {
    if (js == nullptr) {
        return;
    }
    js->onResize(width,height);
}

void VEngine::dispose() {
    if (js == nullptr) {
        return;
    }
    delete js;
    js = nullptr;
}
