#include <v8-function.h>
#include <v8-context.h>


#include "ExternalFunctions.h"
#include "../logger/Logger.h"
#include "../misc/fun.hpp"
#include "../misc/win/env.h"
#include "../misc/win/misc.h"
#include "../misc/win/bitmap/bitmap.h"
#include "GLES2/gl2.h"

Env* envClosure;
BitmapCache bitmapCache;

void setSurfaceWidth(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (!envClosure) return;
    const int width = getNumericParameter<int>(args, 0);
    if (width <= 0 || !IsWindow(envClosure->canvas)) {
        return;
    }
    RECT rect;
    if (!GetWindowRect(envClosure->canvas, &rect)) {
        return;
    }
    const int height = rect.bottom - rect.top;
    SetWindowPos(
        envClosure->canvas,
        nullptr,
        0, 0,
        width, height,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
    );
    CenterChildWindow(envClosure->window, envClosure->canvas);
}

void getSurfaceWidth(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int width = 0;
    if (envClosure) {
        RECT rect;
        GetClientRect(envClosure->canvas, &rect);
        width = rect.right - rect.left;
    }
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),width));
}

void setSurfaceHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (!envClosure) return;
    const int height = getNumericParameter<int>(args, 0);
    if (height <= 0 || !IsWindow(envClosure->canvas)) {
        return;
    }
    RECT rect;
    if (!GetWindowRect(envClosure->canvas, &rect)) {
        return;
    }
    const int width = rect.right - rect.left;
    SetWindowPos(
        envClosure->canvas,
        nullptr,
        0, 0,
        width, height,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
    );
    CenterChildWindow(envClosure->window, envClosure->canvas);
}

void getSurfaceHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int height = 0;
    if (envClosure) {
        RECT rect;
        GetClientRect(envClosure->canvas, &rect);
        height = rect.bottom - rect.top;
    }
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),height));
}

void loadBitmap(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::String::Utf8Value url(args.GetIsolate(), args[0]->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked());
    const auto [success, error, bitmap] = loadBitmap(envClosure, *url);
    int id = 0;
    int width = 0;
    int height = 0;
    if (!success) {
        Logger::error(error);
    }
    else {
        bitmapCache.put(bitmap);
        id = bitmap.id;
        width = bitmap.width;
        height = bitmap.height;
    }
    v8::Local<v8::Object> result = v8::Object::New(args.GetIsolate());
    result->Set(
            args.GetIsolate()->GetCurrentContext(),
            v8::String::NewFromUtf8(args.GetIsolate(), "id").ToLocalChecked(),
            v8::Integer::NewFromUnsigned(args.GetIsolate(), id)
    ).Check();
    result->Set(
            args.GetIsolate()->GetCurrentContext(),
            v8::String::NewFromUtf8(args.GetIsolate(), "width").ToLocalChecked(),
            v8::Integer::NewFromUnsigned(args.GetIsolate(), width)
    ).Check();
    result->Set(
            args.GetIsolate()->GetCurrentContext(),
            v8::String::NewFromUtf8(args.GetIsolate(), "height").ToLocalChecked(),
            v8::Integer::NewFromUnsigned(args.GetIsolate(), height)
    ).Check();
    args.GetReturnValue().Set(result);
}

void loadString(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::String::Utf8Value jsArg0(args.GetIsolate(), args[0]->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked());
    const auto [success, error, content] = loadFileAsString(envClosure, *jsArg0);
    if (!success) {
        Logger::error(error);
    }
    else {
        const v8::Local<v8::String> jsString = v8::String::NewFromUtf8(args.GetIsolate(), content.c_str()).ToLocalChecked();
        args.GetReturnValue().Set(jsString);
    }
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
    auto target = getNumericParameter<int>(args, 0);
    auto level = getNumericParameter<int>(args, 1);
    const auto bitmapId = getNumericParameter<int>(args, 5);
    const auto bitmap = bitmapCache.get(bitmapId);
    if (bitmap == nullptr) {
        Logger::error("wrong bitmap id " + std::to_string(bitmapId));
        return;
    }
    glTexImage2D(
        target,
        level,
        GL_RGBA,
        bitmap->width,
        bitmap->height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        bitmap->pixels.data()
    );
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
