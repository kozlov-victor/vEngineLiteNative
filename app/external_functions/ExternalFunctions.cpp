#include <v8-function.h>
#include <v8-context.h>

#include "ExternalFunctions.h"
#include "../misc/fun.hpp"
#include "external_functions_impl/ExternalFunctionsImpl.h"


void ExternalFunctions::create(void* env, v8::Isolate *isolate, v8::Local<v8::Context> &context_local) {
    ExternalFunctionImpl::createEnvironmentClosure(env);
    const v8::Local<v8::Object> ext = v8::Object::New(isolate);
    const std::vector<Fun> funcs = {
        {"_setSurfaceWidth", ExternalFunctionImpl::setSurfaceWidth},
        {"_getSurfaceWidth", ExternalFunctionImpl::getSurfaceWidth},
        {"_setSurfaceHeight", ExternalFunctionImpl::setSurfaceHeight},
        {"_getSurfaceHeight", ExternalFunctionImpl::getSurfaceHeight},
        {"_loadBitmap", ExternalFunctionImpl::loadBitmap},
        {"_loadString", ExternalFunctionImpl::loadString},
        {"_loadBinary", ExternalFunctionImpl::loadBinary},
        {"_texImage2D_6", ExternalFunctionImpl::texImage2D_6},
        {"_alert", ExternalFunctionImpl::alert},
        {"_setBgColor", ExternalFunctionImpl::setBgColor},
        {"_setTitle", ExternalFunctionImpl::setTitle},
        {"_getTitle", ExternalFunctionImpl::getTitle},
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
