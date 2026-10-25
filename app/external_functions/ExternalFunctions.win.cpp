#include <v8-function.h>
#include <v8-context.h>


#include "ExternalFunctions.h"
#include "../logger/Logger.h"
#include "../misc/fun.hpp"

// MessageBoxW(
//     nullptr,
//     L"Помилка JavaScript. Дивись vengine.log біля EXE.",
//     L"vEngineLite Native",
//     MB_OK | MB_ICONERROR
// );

void setSurfaceWidth(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto width = getNumericParameter<int>(args, 0);
    // todo
}

void getSurfaceWidth(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
    int width = 800;
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),width));
}

void setSurfaceHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
}

void getSurfaceHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
    int height = 600;
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),height));
}

void loadBitmap(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
}

void loadString(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
}

void loadBinary(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
}

void alert(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
}

void texImage2D_6(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
}

void setBgColor(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
}

void ExternalFunctions::create(void* env, v8::Isolate *isolate, v8::Local<v8::Context> &context_local) {
    v8::Local<v8::Object> ext = v8::Object::New(isolate);

    std::vector<Fun> funcs = {
        {"setSurfaceWidth", setSurfaceWidth},
        {"getSurfaceWidth", getSurfaceWidth},
        {"setSurfaceHeight", setSurfaceHeight},
        {"getSurfaceHeight", getSurfaceHeight},
        {"_loadBitmap", loadBitmap},
        {"_loadString", loadString},
        {"_loadBinary", loadBinary},
        {"_texImage2D_6", texImage2D_6},
        {"_alert", alert},
        {"_setBgColor", setBgColor},
    };

    for(const Fun& f : funcs) {
        ext->Set(
            context_local,
            v8::String::NewFromUtf8(isolate, f.name.c_str()).ToLocalChecked(),
            v8::Function::New(context_local, f.value).ToLocalChecked()
        ).Check();
    }

    context_local->Global()->Set(
        context_local,
        v8::String::NewFromUtf8(isolate, "_external").ToLocalChecked(),
        ext
    ).Check();
}
