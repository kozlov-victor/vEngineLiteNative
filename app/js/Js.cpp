#include <string>
#include <sstream>

#include <v8-isolate.h>
#include <v8.h>
#include <v8-version-string.h>

#include <utility>
#include "Js.h"
#include "libplatform/libplatform.h"
#include "../logger/Logger.h"
#include "../console/Console.h"
#include "../gl/GlFields.h"
#include "../gl/GlFunctions.h"
#include "../external_functions/ExternalFunctions.h"
#include "../js_compilation_result/JsCompilationResult.h"


JsCompilationResult reportError(v8::Isolate *isolate,v8::TryCatch &tryCatch,v8::Local<v8::Context> &context,const std::string& error) {
    v8::String::Utf8Value e(isolate, tryCatch.Exception());
    v8::String::Utf8Value trace(isolate, tryCatch.StackTrace(context).ToLocalChecked());
    v8::Local<v8::Message> message = tryCatch.Message();

    std::stringstream ss;

    ss << error << std::endl;
    ss << *e << std::endl;

    if (!message.IsEmpty()) {
        v8::String::Utf8Value filename(isolate,message->GetScriptOrigin().ResourceName());
        ss << "filename: " << *filename << ", ";
        int linenum = message->GetLineNumber(context).FromJust();
        ss << "linenum: " << linenum << ", " << std::endl;
        v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
        ss << "sourceline: " << std::endl << *sourceline << std::endl;
        int start = message->GetStartColumn(context).FromJust();
        for (int i = 0; i < start; i++) {
            ss << " ";
        }
        int end = message->GetEndColumn(context).FromJust();
        for (int i = start; i < end; i++) {
            ss << "^";
        }
        ss << std::endl;
    }

    Logger::error(ss.str());

    JsCompilationResult result(false,ss.str());
    return result;
}

Js::Js() {
    Logger::info("----Js engine created----");
}

void Js::initV8() {
    Logger::info("Initialize V8");
    v8::V8::InitializeICU();
    platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    // Create a new Isolate and make it the current one.
    v8::Isolate::CreateParams create_params;
    allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    create_params.array_buffer_allocator = allocator;
    isolate = v8::Isolate::New(create_params);

    v8::Isolate::Scope isolate_scope(isolate);
    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);
    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate);

    // attach the context to the persistent context, to avoid V8 GC-ing it
    persistentContext = new v8::Persistent<v8::Context>();
    persistentContext->Reset(isolate, context);
    Logger::info("V8 is ready");
    Logger::info("V8 version is");
    Logger::info(V8_VERSION_STRING);

}

void Js::initGlobalObjects(void* env) const {
    Logger::info("init global objects");
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);

    // Enter the context_local for compiling and running the hello world script.
    v8::Local<v8::Context> context_local = v8::Local<v8::Context>::New(isolate, *persistentContext);
    v8::Context::Scope context_scope(context_local);

    // create js global objects
    ExternalFunctions::create(env, isolate, context_local);
    Console::create(isolate,context_local);
    v8::Local<v8::Object> globalGL = v8::Object::New(isolate);
    GlFields::create(isolate, context_local, globalGL);
    GlFunctions::create(isolate, context_local, globalGL);
    context_local->Global()->Set(
            context_local,
            v8::String::NewFromUtf8(isolate, "_globalGL").ToLocalChecked(),
            globalGL
    ).Check();
}

JsCompilationResult Js::compileScript(const char* fileName, const char* code) const {
    Logger::info("script compiling");
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);

    // Enter the context_local for compiling and running the hello world script.
    v8::Local<v8::Context> context_local = v8::Local<v8::Context>::New(isolate, *persistentContext);
    v8::Context::Scope context_scope(context_local);

    // Create a string containing the JavaScript source code.
    v8::Local<v8::String> source = v8::String::NewFromUtf8(
            isolate, code, v8::NewStringType::kNormal).ToLocalChecked();

    v8::TryCatch tryCatch(isolate);

    // Compile the source code.
    v8::ScriptOrigin origin(v8::String::NewFromUtf8(isolate, fileName).ToLocalChecked());

    v8::MaybeLocal<v8::Script> script =
            v8::Script::Compile(context_local, source, &origin);

    if (script.IsEmpty()) {
        return reportError(isolate,tryCatch,context_local,"parsing error");
    } else {
        // Run the script to get the result.
        v8::MaybeLocal<v8::Value> result = script.ToLocalChecked()->Run(context_local);

        if (result.IsEmpty()) {
            return reportError(isolate,tryCatch,context_local,"evaluation error");
        }
        else {
            Logger::info("js script compiled");
            JsCompilationResult success(true,"");
            return success;
        }
    }

}

bool Js::findFunctionByName(
        const char* name,
        v8::Global<v8::Function>& result
) const {
    if (isolate == nullptr ||
        persistentContext == nullptr ||
        persistentContext->IsEmpty() ||
        name == nullptr) {
        return false;
    }

    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::Context> context =
            persistentContext->Get(isolate);

    if (context.IsEmpty()) {
        return false;
    }

    v8::Context::Scope context_scope(context);
    v8::TryCatch try_catch(isolate);

    v8::Local<v8::String> key;

    if (!v8::String::NewFromUtf8(
            isolate,
            name,
            v8::NewStringType::kNormal
    ).ToLocal(&key)) {
        return false;
    }

    v8::Local<v8::Value> value;

    if (!context->Global()->Get(context, key).ToLocal(&value)) {
        Logger::error(
                std::string("findFunctionByName: failed to get ") + name
        );
        return false;
    }

    if (!value->IsFunction()) {
        Logger::error(
                std::string("findFunctionByName: not a function: ") + name
        );
        return false;
    }

    result.Reset(isolate, value.As<v8::Function>());

    return true;
}

bool Js::callFunction(const v8::Global<v8::Function>& function, int argc, v8::Local<v8::Value> argv[]) const {
    if (isolate == nullptr ||
        persistentContext == nullptr ||
        persistentContext->IsEmpty() ||
        function.IsEmpty() ||
        argc < 0 ||
        (argc > 0 && argv == nullptr)) {
        Logger::error("callFunction: invalid arguments or V8 state");
        return false;
    }

    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::Context> context = persistentContext->Get(isolate);

    if (context.IsEmpty()) {
        Logger::error("callFunction: context is empty");
        return false;
    }

    v8::Context::Scope context_scope(context);
    v8::TryCatch try_catch(isolate);

    v8::Local<v8::Function> localFunction = function.Get(isolate);

    v8::Local<v8::Value> result;

    if (!localFunction->Call(
            context,
            context->Global(),
            argc,
            argv
    ).ToLocal(&result)) {

        Logger::error("callFunction: JavaScript function failed");

        if (try_catch.HasCaught()) {
            v8::String::Utf8Value exception(
                    isolate,
                    try_catch.Exception()
            );

            if (*exception != nullptr) {
                Logger::error(*exception);
            }
        }

        return false;
    }

    return true;
}

void Js::onResize(int width, int height) const {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Handle<v8::Context> context_local =
            v8::Local<v8::Context>::New(isolate, persistentContext->Get(isolate));
    v8::Context::Scope context_scope(context_local);
    v8::TryCatch tryCatch(isolate);

    context_local->Global()->Set(
        context_local,
        v8::String::NewFromUtf8(isolate,"innerWidth").ToLocalChecked(),
        v8::Integer::New(isolate,width)
    ).Check();
    context_local->Global()->Set(
        context_local,
        v8::String::NewFromUtf8(isolate,"innerHeight").ToLocalChecked(),
        v8::Integer::New(isolate,height)
    ).Check();

    v8::Local<v8::Value> argv[] = {
        v8::String::NewFromUtf8(isolate,"resize").ToLocalChecked()
    };
    callFunction(triggerEventFunction,1,argv);
}

void Js::onTouchEvent(const char *event_name, float x, float y, int id) const {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Handle<v8::Context> context_local =
            v8::Local<v8::Context>::New(isolate, persistentContext->Get(isolate));
    v8::Context::Scope context_scope(context_local);
    v8::TryCatch tryCatch(isolate);

    v8::Local<v8::Object> globalGl =
        context_local->Global()->Get(context_local, v8::String::NewFromUtf8(isolate, "_globalGL").ToLocalChecked()).
        ToLocalChecked().As<v8::Object>();
    v8::Local<v8::Object> canvas =
            globalGl->Get(context_local, v8::String::NewFromUtf8(isolate, "canvas").ToLocalChecked()).
            ToLocalChecked().As<v8::Object>();
    v8::Local<v8::Object> event_fn =
        canvas->Get(context_local, v8::String::NewFromUtf8(isolate, event_name).ToLocalChecked()).
        ToLocalChecked().As<v8::Object>();

    if (!event_fn->IsFunction()) {
        Logger::error("not a function");
        Logger::error(event_name);
        return;
    };

    v8::Local<v8::Object> touch_object = v8::Object::New(isolate);
    touch_object->Set(
        context_local,
        v8::String::NewFromUtf8(isolate, "clientX").ToLocalChecked(),
        v8::Number::New(isolate,x)
    ).Check();
    touch_object->Set(
        context_local,
        v8::String::NewFromUtf8(isolate, "clientY").ToLocalChecked(),
        v8::Number::New(isolate,y)
    ).Check();
    touch_object->Set(
        context_local,
        v8::String::NewFromUtf8(isolate, "pointerId").ToLocalChecked(),
        v8::Integer::New(isolate,id)
    ).Check();
    v8::Local<v8::Array> touche_object_array = v8::Array::New(isolate, 1);
    touche_object_array->Set(context_local, 0, touch_object).Check();

    v8::Local<v8::Object> event = v8::Object::New(isolate);
    event->Set(
        context_local,
        v8::String::NewFromUtf8(isolate, "preventDefault").ToLocalChecked(),
        v8::Function::New(context_local,[](auto){}).ToLocalChecked()
    ).Check();
    event->Set(
        context_local,
        v8::String::NewFromUtf8(isolate, "touches").ToLocalChecked(),
        touche_object_array
    ).Check();
    event->Set(
        context_local,
        v8::String::NewFromUtf8(isolate, "changedTouches").ToLocalChecked(),
        touche_object_array
    ).Check();

    v8::Local<v8::Value> argv[] = {event};
    v8::MaybeLocal<v8::Value> result =
        event_fn->CallAsFunction(context_local, context_local->Global(), 1, argv);
    if (result.IsEmpty()) {
        reportError(isolate,tryCatch,context_local,"function call error");
    }

}

Js::~Js() {
    // Спершу треба зупинити всі потоки, які можуть звертатися до V8,
    // і дочекатися завершення їхньої роботи.

    if (persistentContext != nullptr) {
        persistentContext->Reset();
        delete persistentContext;
        persistentContext = nullptr;
    }

    if (isolate != nullptr) {
        isolate->Dispose();
        isolate = nullptr;
        nextTickFunction.Reset();
        triggerEventFunction.Reset();
    }

    if (allocator != nullptr) {
        delete allocator;
        allocator = nullptr;
    }

    // Це глобальні операції: виконуй їх лише тоді,
    // коли інші Isolate у процесі більше не використовуються.
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
}
