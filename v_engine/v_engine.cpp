#include "v_engine.h"

#include <iostream>
#include <v8-version-string.h>
#include "../app/misc/win/env.h"
#include "../app/bootstrap/bootstrap.hpp"
#include "../app/misc/win/misc.h"

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

JsCompilationResult VEngine::compileScriptFromUrl(const std::string &url) {
    auto [success, error, content] = loadFileAsString(env, url);
    if (!success) return {false, error};
    return compileScript(url, content);
}

void VEngine::updateFrame() const {
    if (js == nullptr) {
        return;
    }
    js->callFunction(js->nextTickFunction);
}

void VEngine::onTouchEvent(std::string eventName, float x, float y, int id) const {
    if (js == nullptr) {
        return;
    }
    js->onTouchEvent(eventName.c_str(),x,y,id);
}

void VEngine::onResize(int width, int height, bool triggerResize) const {
    if (js == nullptr) {
        return;
    }
    js->onResize(width,height,triggerResize);
}

void VEngine::dispose() {
    if (js == nullptr) {
        return;
    }
    delete js;
    js = nullptr;
}
