#pragma once

#include <filesystem>
#include <memory>

#include <v8.h>
#include <libplatform/libplatform.h>

class JsRuntime final
{
public:
    JsRuntime() = default;
    ~JsRuntime();

    JsRuntime(const JsRuntime&) = delete;
    JsRuntime& operator=(const JsRuntime&) = delete;

    bool Initialize(
        const std::filesystem::path& scriptPath,
        const std::filesystem::path& executablePath
    );

    // timestampMs — час від початку роботи рушія в мілісекундах.
    bool Tick(double timestampMs);

    void Shutdown();

private:
    static void ConsoleLogCallback(
        const v8::FunctionCallbackInfo<v8::Value>& info
    );

    static void ReportException(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        v8::TryCatch& tryCatch,
        const char* phase
    );

private:
    std::unique_ptr<v8::Platform> platform_;

    v8::Isolate* isolate_ = nullptr;
    v8::ArrayBuffer::Allocator* allocator_ = nullptr;

    v8::Global<v8::Context> context_;
    v8::Global<v8::Function> requestAnimationFrame_;

    bool platformInitialized_ = false;
    bool v8Initialized_ = false;
};