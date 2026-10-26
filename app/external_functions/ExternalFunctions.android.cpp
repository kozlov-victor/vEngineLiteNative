#include <v8-function.h>
#include <v8-context.h>

#include <jni.h>
#include "ExternalFunctions.h"
#include "../logger/Logger.h"
#include "../misc/fun.hpp"
#include "../misc/android/misc.h"

JNIEnv *envClosure;


void setSurfaceWidth(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto width = getNumericParameter<int>(args, 0);
    jclass cl = envClosure->FindClass("com/vengine/engine/VEngine");
    jmethodID cid = envClosure->GetStaticMethodID(cl, "setSurfaceWidth", "(I)V");
    envClosure->CallStaticVoidMethod(cl, cid, width);
}

void getSurfaceWidth(const v8::FunctionCallbackInfo<v8::Value>& args) {
    jclass cl = envClosure->FindClass("com/vengine/engine/VEngine");
    jmethodID cid = envClosure->GetStaticMethodID(cl, "getSurfaceWidth", "()I");
    jint width = envClosure->CallStaticIntMethod(cl, cid);
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),width));
}

void setSurfaceHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto height = getNumericParameter<int>(args, 0);
    jclass cl = envClosure->FindClass("com/vengine/engine/VEngine");
    jmethodID cid = envClosure->GetStaticMethodID(cl, "setSurfaceHeight", "(I)V");
    envClosure->CallStaticVoidMethod(cl, cid, height);
}

void getSurfaceHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
    jclass cl = envClosure->FindClass("com/vengine/engine/VEngine");
    jmethodID cid = envClosure->GetStaticMethodID(cl, "getSurfaceHeight", "()I");
    jint height = envClosure->CallStaticIntMethod(cl, cid);
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),height));
}

void loadBitmap(const v8::FunctionCallbackInfo<v8::Value>& args) {
    jclass cl = envClosure->FindClass("com/vengine/engine/VEngine");
    jmethodID m = envClosure->GetStaticMethodID(cl, "loadBitmap", "(Ljava/lang/String;)Lcom/vengine/model/BitmapInfo;");
    v8::String::Utf8Value str(args.GetIsolate(), args[0]->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked());
    jstring jStr = envClosure->NewStringUTF(*str);
    jobject jObj = envClosure->CallStaticObjectMethod(cl, m, jStr); // class bitmapInfo

    jclass bitmapInfoCl = envClosure->FindClass("com/vengine/model/BitmapInfo");
    jmethodID getIdMtd = envClosure->GetMethodID(bitmapInfoCl,"getId", "()I");
    jmethodID getWidthMtd = envClosure->GetMethodID(bitmapInfoCl,"getWidth", "()I");
    jmethodID getHeightMtd = envClosure->GetMethodID(bitmapInfoCl,"getHeight", "()I");

    jint id = envClosure->CallIntMethod(jObj,getIdMtd);
    jint width = envClosure->CallIntMethod(jObj,getWidthMtd);
    jint height = envClosure->CallIntMethod(jObj,getHeightMtd);

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
    jclass cl = envClosure->FindClass("com/vengine/engine/VEngine");
    jmethodID m = envClosure->GetStaticMethodID(cl, "loadString", "(Ljava/lang/String;)Ljava/lang/String;");
    v8::String::Utf8Value jsArg0(args.GetIsolate(), args[0]->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked());
    jstring jArg0 = envClosure->NewStringUTF(*jsArg0);
    auto jResult = static_cast<jstring>(envClosure->CallStaticObjectMethod(cl, m, jArg0));
    std::string cResult = jstring2string(envClosure, jResult);
    v8::Local<v8::String> jsString = v8::String::NewFromUtf8(args.GetIsolate(), cResult.c_str()).ToLocalChecked();
    args.GetReturnValue().Set(jsString);
}

void loadBinary(const v8::FunctionCallbackInfo<v8::Value>& args) {
    jclass cl = envClosure->FindClass("com/vengine/engine/VEngine");
    jmethodID m = envClosure->GetStaticMethodID(cl, "loadBinary", "(Ljava/lang/String;)Ljava/nio/ByteBuffer;");
    v8::String::Utf8Value jsArg0(args.GetIsolate(), args[0]->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked());
    jstring jArg0 = envClosure->NewStringUTF(*jsArg0);
    jobject jByteBuffer = envClosure->CallStaticObjectMethod(cl, m, jArg0);

    char *buf = (char*)envClosure->GetDirectBufferAddress(jByteBuffer);
    jlong capacity = envClosure->GetDirectBufferCapacity(jByteBuffer);
    v8::Handle<v8::ArrayBuffer> jsArrayBuffer = v8::ArrayBuffer::New(args.GetIsolate(),  capacity);
    memcpy(
            jsArrayBuffer->GetBackingStore()->Data(),
            buf,
            static_cast<size_t>(capacity)
    );
    args.GetReturnValue().Set(jsArrayBuffer);
}

void alert(const v8::FunctionCallbackInfo<v8::Value>& args) {
    jclass cl = envClosure->FindClass("com/vengine/engine/VEngine");
    jmethodID m = envClosure->GetStaticMethodID(cl, "alert", "(Ljava/lang/String;)V");
    v8::String::Utf8Value jsArg0(args.GetIsolate(), args[0]->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked());
    jstring jArg0 = envClosure->NewStringUTF(*jsArg0);
    envClosure->CallStaticVoidMethod(cl,m, jArg0);
}

void texImage2D_6(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto target = getNumericParameter<int>(args, 0);
    auto level = getNumericParameter<int>(args, 1);
    auto bitmap = getNumericParameter<int>(args, 5);
    jclass cl = envClosure->FindClass("com/vengine/engine/VEngine");
    jmethodID m = envClosure->GetStaticMethodID(cl, "getBitmap", "(III)V");
    envClosure->CallStaticVoidMethod(cl,m,target,level,bitmap);
}

void setBgColor(const v8::FunctionCallbackInfo<v8::Value>& args) {
    jclass cl = envClosure->FindClass("com/vengine/engine/VEngine");
    jmethodID m = envClosure->GetStaticMethodID(cl, "setBgColor", "(Ljava/lang/String;)V");
    v8::String::Utf8Value jsArg0(args.GetIsolate(), args[0]->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked());
    jstring jArg0 = envClosure->NewStringUTF(*jsArg0);
    envClosure->CallStaticVoidMethod(cl, m, jArg0);
}

void setTitle(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Logger::warn("setTitle not implemented for android platform");
}

void ExternalFunctions::create(void* env, v8::Isolate *isolate, v8::Local<v8::Context> &context_local) {
    v8::Local<v8::Object> ext = v8::Object::New(isolate);
    envClosure = static_cast<JNIEnv*>(env);

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
