
#ifndef VENGINELITE_EXTERNALFUNCTIONSIMPL_H
#define VENGINELITE_EXTERNALFUNCTIONSIMPL_H

#include <v8-function.h>

namespace ExternalFunctionImpl {
    void createEnvironmentClosure(void* env);
    void setSurfaceWidth(const v8::FunctionCallbackInfo<v8::Value>& args);
    void getSurfaceWidth(const v8::FunctionCallbackInfo<v8::Value>& args);
    void setSurfaceHeight(const v8::FunctionCallbackInfo<v8::Value>& args);
    void getSurfaceHeight(const v8::FunctionCallbackInfo<v8::Value>& args);
    void loadBitmap(const v8::FunctionCallbackInfo<v8::Value>& args);
    void loadString(const v8::FunctionCallbackInfo<v8::Value>& args);
    void loadBinary(const v8::FunctionCallbackInfo<v8::Value>& args);
    void texImage2D_6(const v8::FunctionCallbackInfo<v8::Value>& args);
    void alert(const v8::FunctionCallbackInfo<v8::Value>& args);
    void setBgColor(const v8::FunctionCallbackInfo<v8::Value>& args);
    void setTitle(const v8::FunctionCallbackInfo<v8::Value>& args);
    void getTitle(const v8::FunctionCallbackInfo<v8::Value>& args);
}


#endif