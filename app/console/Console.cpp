
#include <v8-function.h>
#include <v8-context.h>
#include <sstream>
#include "Console.h"
#include "../logger/Logger.h"

void _console_log_(const int level,const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate *isolate = args.GetIsolate();
    std::stringstream ss;
    for (int i=0;i<args.Length();i++) {
        v8::String::Utf8Value str(isolate, args[i]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
        ss << *str;
        if (i<args.Length()-1) ss << " ";
    }
    switch (level) {
        case 0:
            Logger::info(ss.str());
            break;
        case 1:
            Logger::warn(ss.str());
            break;
        default:
            Logger::error(ss.str());
    }
}

void Console::create(v8::Isolate *isolate,v8::Local<v8::Context> &context) {
    const v8::Local<v8::Object> console = v8::Object::New(isolate);
    console->Set(
            context,
            v8::String::NewFromUtf8(isolate, "_log").ToLocalChecked(),
            v8::Function::New(context,
              [](auto args) {
                  _console_log_(0,args);
              }).ToLocalChecked()
    ).Check();
    console->Set(
            context,
            v8::String::NewFromUtf8(isolate, "_warn").ToLocalChecked(),
            v8::Function::New(context,
              [](auto args) {
                  _console_log_(1,args);
              }).ToLocalChecked()
    ).Check();
    console->Set(
            context,
            v8::String::NewFromUtf8(isolate, "_error").ToLocalChecked(),
            v8::Function::New(context,
            [](auto args) {
                _console_log_(2,args);
            }).ToLocalChecked()
    ).Check();
    context->Global()->Set(
            context,
            v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(),
            console
    ).Check();
}
