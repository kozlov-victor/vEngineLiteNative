#include <v8-function.h>
#include <v8-context.h>


#include "ExternalFunctions.h"
#include "../logger/Logger.h"
#include "../misc/fun.hpp"
#include "../misc/win/env.h"
#include "../misc/win/misc.h"

Env* envClosure;

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
    if (args.Length() < 2) {
        return;
    }
    v8::String::Utf8Value message(args.GetIsolate(), args[0]->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked());
    v8::String::Utf8Value title(args.GetIsolate(), args[1]->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked());
    MessageBoxW(
        nullptr,
        jsStrToWStr(message).c_str(),
        jsStrToWStr(title).c_str(),
        MB_OK | MB_ICONERROR
    );
}

void texImage2D_6(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
}

void setBgColor(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // todo
}

void setTitle(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() < 1) {
        return;
    }
    v8::String::Utf8Value jsArg0(args.GetIsolate(), args[0]->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked());
    std::wstring title = jsStrToWStr(jsArg0);
    SetWindowTextW(envClosure->window, title.c_str());
}

void ExternalFunctions::create(void* env, v8::Isolate *isolate, v8::Local<v8::Context> &context_local) {
    envClosure = static_cast<Env*>(env);
    v8::Local<v8::Object> ext = v8::Object::New(isolate);

    std::vector<Fun> funcs = {
        {"_setSurfaceWidth", setSurfaceWidth},
        {"_getSurfaceWidth", getSurfaceWidth},
        {"_setSurfaceHeight", setSurfaceHeight},
        {"_getSurfaceHeight", getSurfaceHeight},
        {"_loadBitmap", loadBitmap},
        {"_loadString", loadString},
        {"_loadBinary", loadBinary},
        {"_texImage2D_6", texImage2D_6},
        {"_alert", alert},
        {"_setBgColor", setBgColor},
        {"_setTitle", setTitle},
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
