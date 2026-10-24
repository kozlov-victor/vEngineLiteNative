#include "JsRuntime.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <string>

JsRuntime::~JsRuntime()
{
    Shutdown();
}

bool JsRuntime::Initialize(
    const std::filesystem::path& scriptPath,
    const std::filesystem::path& executablePath)
{
    Shutdown();

    // 1. Read the script.
    std::ifstream file(scriptPath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Cannot open JavaScript file: "
                  << scriptPath.string() << '\n';
        return false;
    }

    std::string source{
        std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}
    };

    if (source.size() >
        static_cast<size_t>(std::numeric_limits<int>::max())) {
        std::cerr << "JavaScript file is too large.\n";
        return false;
    }

    const std::string exePath = executablePath.string();

    // 2. Initialize V8 and its platform.
    if (!v8::V8::InitializeICUDefaultLocation(exePath.c_str())) {
        std::cerr << "V8 ICU initialization failed.\n";
        return false;
    }

    v8::V8::InitializeExternalStartupData(exePath.c_str());

    platform_ = v8::platform::NewDefaultPlatform();

    if (!platform_) {
        std::cerr << "Cannot create V8 platform.\n";
        return false;
    }

    v8::V8::InitializePlatform(platform_.get());
    platformInitialized_ = true;

    if (!v8::V8::Initialize()) {
        std::cerr << "V8 initialization failed.\n";
        Shutdown();
        return false;
    }

    v8Initialized_ = true;

    // 3. Create an isolate.
    allocator_ = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    if (!allocator_) {
        std::cerr << "Cannot create V8 memory allocator.\n";
        Shutdown();
        return false;
    }

    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = allocator_;

    isolate_ = v8::Isolate::New(params);

    if (!isolate_) {
        std::cerr << "Cannot create V8 isolate.\n";
        Shutdown();
        return false;
    }

    // Scopes must finish before Shutdown() disposes the isolate.
    const bool success = [&]() -> bool {
        v8::Isolate::Scope isolateScope(isolate_);
        v8::HandleScope handleScope(isolate_);

        v8::Local<v8::Context> context =
            v8::Context::New(isolate_);

        context_.Reset(isolate_, context);

        v8::Context::Scope contextScope(context);
        v8::TryCatch tryCatch(isolate_);

        // 4. Expose console.log to JavaScript.
        v8::Local<v8::Object> console = v8::Object::New(isolate_);

        v8::Local<v8::Function> logFunction;

        if (!v8::Function::New(
                context,
                ConsoleLogCallback
            ).ToLocal(&logFunction)) {
            std::cerr << "Cannot create console.log.\n";
            return false;
        }

        if (!console->Set(
                context,
                v8::String::NewFromUtf8Literal(isolate_, "log"),
                logFunction
            ).FromMaybe(false)) {
            std::cerr << "Cannot configure console.log.\n";
            return false;
        }

        if (!context->Global()->Set(
                context,
                v8::String::NewFromUtf8Literal(isolate_, "console"),
                console
            ).FromMaybe(false)) {
            std::cerr << "Cannot expose console to JavaScript.\n";
            return false;
        }

        // 5. Compile and execute index.js once.
        v8::Local<v8::String> jsSource;

        if (!v8::String::NewFromUtf8(
                isolate_,
                source.c_str(),
                v8::NewStringType::kNormal,
                static_cast<int>(source.size())
            ).ToLocal(&jsSource)) {
            std::cerr << "Cannot create V8 source string.\n";
            return false;
        }

        v8::Local<v8::Script> script;

        if (!v8::Script::Compile(context, jsSource)
                 .ToLocal(&script)) {
            ReportException(
                isolate_, context, tryCatch, "Compiling index.js"
            );
            return false;
        }

        if (script->Run(context).IsEmpty()) {
            ReportException(
                isolate_, context, tryCatch, "Executing index.js"
            );
            return false;
        }

        // 6. Find the global requestAnimationFrame function.
        v8::Local<v8::Value> callback;

        if (!context->Global()->Get(
                context,
                v8::String::NewFromUtf8Literal(
                    isolate_, "requestAnimationFrame"
                )
            ).ToLocal(&callback)) {
            ReportException(
                isolate_, context, tryCatch,
                "Finding requestAnimationFrame"
            );
            return false;
        }

        if (!callback->IsFunction()) {
            std::cerr
                << "index.js must define a global function "
                   "requestAnimationFrame(timestamp).\n";
            return false;
        }

        requestAnimationFrame_.Reset(
            isolate_,
            callback.As<v8::Function>()
        );

        return true;
    }();

    if (!success) {
        Shutdown();
        return false;
    }

    std::cout << "V8 initialized. index.js loaded.\n";
    return true;
}

bool JsRuntime::Tick(double timestampMs)
{
    if (!isolate_ ||
        context_.IsEmpty() ||
        requestAnimationFrame_.IsEmpty()) {
        return false;
    }

    v8::Isolate::Scope isolateScope(isolate_);
    v8::HandleScope handleScope(isolate_);

    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope contextScope(context);
    v8::TryCatch tryCatch(isolate_);

    v8::Local<v8::Function> callback =
        requestAnimationFrame_.Get(isolate_);

    v8::Local<v8::Value> args[] = {
        v8::Number::New(isolate_, timestampMs)
    };

    v8::Local<v8::Value> result;

    if (!callback->Call(
            context,
            context->Global(),
            1,
            args
        ).ToLocal(&result)) {
        ReportException(
            isolate_, context, tryCatch,
            "requestAnimationFrame"
        );
        return false;
    }

    return true;
}

void JsRuntime::ConsoleLogCallback(
    const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate* isolate = info.GetIsolate();

    for (int i = 0; i < info.Length(); ++i) {
        if (i > 0) {
            std::cout << ' ';
        }

        v8::String::Utf8Value text(isolate, info[i]);

        if (*text) {
            std::cout << *text;
        } else {
            std::cout << "<value>";
        }
    }

    std::cout << '\n';
}

void JsRuntime::ReportException(
    v8::Isolate* isolate,
    v8::Local<v8::Context> context,
    v8::TryCatch& tryCatch,
    const char* phase)
{
    std::cerr << "[JavaScript] " << phase << ": ";

    v8::String::Utf8Value exception(
        isolate, tryCatch.Exception()
    );

    if (*exception) {
        std::cerr << *exception;
    } else {
        std::cerr << "unknown exception";
    }

    v8::Local<v8::Message> message = tryCatch.Message();

    if (!message.IsEmpty()) {
        const int line = message->GetLineNumber(context)
                             .FromMaybe(-1);

        if (line > 0) {
            std::cerr << " (line " << line << ')';
        }
    }

    std::cerr << '\n';
}

void JsRuntime::Shutdown()
{
    // Release persistent handles before disposing the isolate.
    requestAnimationFrame_.Reset();
    context_.Reset();

    if (isolate_) {
        isolate_->Dispose();
        isolate_ = nullptr;
    }

    delete allocator_;
    allocator_ = nullptr;

    if (v8Initialized_) {
        v8::V8::Dispose();
        v8Initialized_ = false;
    }

    if (platformInitialized_) {
        v8::V8::DisposePlatform();
        platformInitialized_ = false;
    }

    platform_.reset();
}