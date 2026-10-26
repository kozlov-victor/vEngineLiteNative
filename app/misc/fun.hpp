
#ifndef VENGINE_FUN_HPP
#define VENGINE_FUN_HPP

#include <string>
#include <v8.h>
#include <v8-function-callback.h>
#include "../logger/Logger.h"

template <typename T>
struct FileContent {
    bool success = true;
    std::string error;
    T content;
};

template <typename T>
T getNumericParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    v8::Isolate* isolate = args.GetIsolate();
    if (i < 0 || i >= args.Length()) {
        Logger::error("wrong parameter index",i);
        return T();
    }
    v8::Local<v8::Value> value = args[i];
    if (!value->IsNumber()) {
        Logger::error("not a numeric parameter");
        return T();
    }
    return static_cast<T>(value->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value());
};

struct Fun {
    std::string name;
    void (*value)(const v8::FunctionCallbackInfo<v8::Value>&);
};
#endif
