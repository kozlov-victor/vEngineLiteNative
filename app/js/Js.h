#ifndef VENGINE_JS_H
#define VENGINE_JS_H

#include <string>
#include <libplatform/libplatform.h>
#include <v8.h>
#include "../js_compilation_result/JsCompilationResult.h"

class Js {

public:
      std::unique_ptr<v8::Platform> platform;
      v8::Isolate *isolate;
      v8::Persistent<v8::Context> *persistentContext;
      v8::ArrayBuffer::Allocator *allocator;
      v8::Global<v8::Function> nextTickFunction;
      v8::Global<v8::Function> triggerEventFunction;
      bool findFunctionByName(const char* name, v8::Global<v8::Function>& result) const;

      Js();
      ~Js();
      void initV8();
      void initGlobalObjects(void* env) const;
      JsCompilationResult compileScript(const char* fileName, const char* code) const;
      bool callFunction(const v8::Global<v8::Function>& function,int argc = 0,v8::Local<v8::Value> argv[] = nullptr) const;
      void onResize(int width, int height, bool triggerResize) const;
      void onTouchEvent(const char* event_name, float x, float y, int id) const;
};


#endif
