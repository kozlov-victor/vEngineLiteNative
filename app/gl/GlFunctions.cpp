#include <v8-typed-array.h>
#include <v8-function.h>
#include <cstdint>
#include <GLES2/gl2.h>

#include "GlFunctions.h"
#include "../logger/Logger.h"
#include "../misc/fun.hpp"


bool isArrayBuffer(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    v8::Isolate* isolate = args.GetIsolate();
    if (i < 0 || i >= args.Length()) {
        Logger::error("wrong index");
        return {};
    }
    v8::Local<v8::Value> arg = args[i];
    if (arg->IsArrayBuffer()) {
        return true;
    } else if (arg->IsArrayBufferView()) {
        return true;
    } else if (arg->IsTypedArray()) {
        return true;
    } else {
        return false;
    }
}


GLuint getIdFromV8GlObject(const v8::FunctionCallbackInfo<v8::Value>& args, int index) {
    if (index < 0 || index >= args.Length()) {
        Logger::error("wrong index");
        return {};
    }

    v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(args[index]);
    if (!obj->IsObject()) return 0;


    v8::Local<v8::String> idKey = v8::String::NewFromUtf8(args.GetIsolate(), "$id", v8::NewStringType::kInternalized).ToLocalChecked();

    if (obj->HasOwnProperty(args.GetIsolate()->GetCurrentContext(), idKey).FromMaybe(false)) {
        v8::Local<v8::Value> idValue = obj->Get(args.GetIsolate()->GetCurrentContext(), idKey).ToLocalChecked();
        v8::MaybeLocal<v8::Uint32> maybeId = idValue->ToUint32(args.GetIsolate()->GetCurrentContext());
        if (!maybeId.IsEmpty()) {
            return maybeId.ToLocalChecked()->Value();
        } else {
            Logger::error("Can not convert $id to Uint32.");
            return 0;
        }
    } else {
        Logger::error("no $id key in object by index",index);
        return 0;
    }
}

std::string getStringParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int index) {
    if (index < 0 || index >= args.Length()) {
        Logger::error("bad parameter index", index);
        return "";
    }

    v8::Local<v8::Value> value = args[index];
    v8::Local<v8::Context> context = args.GetIsolate()->GetCurrentContext();

    v8::Local<v8::String> stringValue;
    if (!value->ToString(context).ToLocal(&stringValue)) {
        Logger::error("failed to convert to string");
        return "";
    }

    v8::String::Utf8Value utf8Value(args.GetIsolate(), stringValue);
    const char* strData = *utf8Value;
    return {strData};
}

v8::Local<v8::Object> createV8GlObjectFromId(const v8::FunctionCallbackInfo<v8::Value>& args, GLuint id) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);

    v8::Local<v8::String> idKey = v8::String::NewFromUtf8(isolate, "$id", v8::NewStringType::kInternalized).ToLocalChecked();
    v8::Local<v8::Value> idValue = v8::Integer::NewFromUnsigned(isolate, id);
    obj->Set(context, idKey, idValue).Check();

    return obj;
}

GLfloat getGlFloatParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    return getNumericParameter<GLfloat>(args, i);
}

GLbitfield getBitfieldParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    return getNumericParameter<GLbitfield>(args, i);
}

GLuint getGlUIntParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    return getNumericParameter<GLuint>(args, i);
}

GLint getGlIntParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    return getNumericParameter<GLint>(args, i);
}

GLboolean getGlBooleanParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::Local<v8::Value> value = args[i];
    if (value->IsBoolean()) {
        return static_cast<GLboolean>(value->BooleanValue(isolate));
    } else if (value->IsNumber()) {
        int intValue = value->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
        return static_cast<GLboolean>(intValue != 0);
    } else {
        Logger::error("parameter neither boolean not number");
        return GL_FALSE;
    }
}

GLenum getGlEnumParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    return getNumericParameter<GLenum>(args, i);
}

GLintptr getGlIntPtrParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    return getNumericParameter<GLintptr>(args, i);
}

GLsizeiptr getGlSizeParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    return getNumericParameter<GLsizeiptr>(args, i);
}

GLsizei getGlSizeiParameter(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    return getNumericParameter<GLsizei>(args, i);
}

v8::Local<v8::ArrayBuffer> getArrayBuffer(const v8::FunctionCallbackInfo<v8::Value>& args, int i) {
    v8::Isolate* isolate = args.GetIsolate();
    if (i < 0 || i >= args.Length()) {
        Logger::error("wrong index");
        return {};
    }
    v8::Local<v8::Value> arg = args[i];
    if (arg->IsArrayBuffer()) {
        return arg.As<v8::ArrayBuffer>();
    } else if (arg->IsArrayBufferView()) {
        v8::Local<v8::ArrayBufferView> bufView = arg.As<v8::ArrayBufferView>();
        return bufView->Buffer();
    } else if (arg->IsTypedArray()) {
        v8::Local<v8::TypedArray> typedArray = arg.As<v8::TypedArray>();
        return typedArray->Buffer();
    } else {
        Logger::error("neither an ArrayBuffer not ArrayBufferView not TypedArray.");
        v8::String::Utf8Value str(isolate, arg->ToString(isolate->GetCurrentContext()).ToLocalChecked());
        Logger::error(*str);
        return {};
    }
}


void activeTexture(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum tex = getGlEnumParameter(args,0);
    glActiveTexture(tex);
}

void attachShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    GLuint shader = getIdFromV8GlObject(args,1);
    glAttachShader(program, shader);
}

void bindAttribLocation(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    GLuint index = getGlUIntParameter(args,1);
    std::string name = getStringParameter(args,2);
    glBindAttribLocation(program, index, name.c_str());
}

void bindBuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlUIntParameter(args, 0);
    GLuint buffer = getIdFromV8GlObject(args,1);
    glBindBuffer(target, buffer);
}

void bindFramebuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint target = getGlUIntParameter(args, 0);
    GLuint buffer = getIdFromV8GlObject(args,1);
    glBindFramebuffer(target, buffer);
}

void bindRenderbuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint target = getGlUIntParameter(args, 0);
    GLuint buffer = getIdFromV8GlObject(args,1);
    glBindRenderbuffer(target, buffer);
}

void bindTexture(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint target = getGlUIntParameter(args, 0);
    GLuint texture = getIdFromV8GlObject(args,1);
    glBindTexture(target, texture);
}

void blendColor(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLfloat r = getGlFloatParameter(args, 0);
    GLfloat g = getGlFloatParameter(args, 1);
    GLfloat b = getGlFloatParameter(args, 2);
    GLfloat a = getGlFloatParameter(args, 3);
    glBlendColor(r,g,b,a);
}

void blendEquation(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum mode = getGlEnumParameter(args, 0);
    glBlendEquation(mode);
}

void blendEquationSeparate(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum modeRGB = getGlEnumParameter(args, 0);
    GLenum modeAlpha = getGlEnumParameter(args, 1);
    glBlendEquationSeparate(modeRGB,modeAlpha);
}

void blendFunc(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum sfactor = getGlEnumParameter(args, 0);
    GLenum dfactor = getGlEnumParameter(args, 1);
    glBlendFunc(sfactor,dfactor);
}

void blendFuncSeparate(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum sfactorRGB = getGlEnumParameter(args, 0);
    GLenum dfactorRGB = getGlEnumParameter(args, 1);
    GLenum sfactorAlpha = getGlEnumParameter(args, 2);
    GLenum dfactorAlpha = getGlEnumParameter(args, 3);
    glBlendFuncSeparate(sfactorRGB,dfactorRGB,sfactorAlpha,dfactorAlpha);
}

void bufferData(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args, 0);
    GLenum usage = getGlEnumParameter(args, 2);

    if (args[1]->IsArrayBuffer() || args[1]->IsArrayBufferView()) {
        void* data = nullptr;
        size_t byteLength = 0;

        if (args[1]->IsArrayBuffer()) {
            auto buffer = args[1].As<v8::ArrayBuffer>();
            auto backingStore = buffer->GetBackingStore();

            data = backingStore->Data();
            byteLength = backingStore->ByteLength();
        } else {
            auto view = args[1].As<v8::ArrayBufferView>();
            auto buffer = view->Buffer();
            auto backingStore = buffer->GetBackingStore();

            data = static_cast<uint8_t*>(backingStore->Data())
                   + view->ByteOffset();
            byteLength = view->ByteLength();
        }

        auto size = static_cast<GLsizeiptr>(byteLength);
        glBufferData(target, size, data, usage);
    } else {
        GLsizeiptr size = getGlSizeParameter(args, 1);
        glBufferData(target, size, nullptr, usage);
    }
}

void bufferSubData(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args, 0);
    GLintptr offset = getGlIntPtrParameter(args, 1);

    v8::Local<v8::ArrayBuffer> buf_data = getArrayBuffer(args, 2);
    auto backingStore = buf_data->GetBackingStore();

    auto size = static_cast<GLsizeiptr>(backingStore->ByteLength());
    void* data = backingStore->Data();

    glBufferSubData(target, offset, size, data);
}

void checkFramebufferStatus(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args, 0);
    GLenum status = glCheckFramebufferStatus(target);
    args.GetReturnValue().Set(v8::Integer::NewFromUnsigned(args.GetIsolate(), status));
}

void clear(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLbitfield mask = getBitfieldParameter(args,0);
    glClear(mask);
}

void clearColor(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLfloat r = getGlFloatParameter(args, 0);
    GLfloat g = getGlFloatParameter(args, 1);
    GLfloat b = getGlFloatParameter(args, 2);
    GLfloat a = getGlFloatParameter(args, 3);
    glClearColor(r,g,b,a);
}

void clearStencil(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLint s = getGlIntParameter(args,0);
    glClearStencil(s);
}

void clearDepth(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLfloat s = getGlFloatParameter(args,0);
    glClearDepthf(s);
}

void colorMask(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLboolean r = getGlBooleanParameter(args,0);
    GLboolean g = getGlBooleanParameter(args,1);
    GLboolean b = getGlBooleanParameter(args,2);
    GLboolean a = getGlBooleanParameter(args,3);
    glColorMask(r,g,b,a);
}

void compileShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint shader = getIdFromV8GlObject(args, 0);
    glCompileShader(shader);
}

// c function void compressedTexImage2D ( gLenum target, gLint level, gLenum internalformat, gLsizei width, gLsizei height, gLint border, gLsizei imageSize, const gLvoid *data )
// not implemented in webgl

// c function void compressedTexSubImage2D ( gLenum target, gLint level, gLint xoffset, gLint yoffset, gLsizei width, gLsizei height, gLenum format, gLsizei imageSize, const gLvoid *data )
// not implemented webgl

void copyTexImage2D(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLint level = getGlIntParameter(args,1);
    GLenum internalformat = getGlEnumParameter(args,2);
    GLint x = getGlIntParameter(args,3);
    GLint y = getGlIntParameter(args,4);
    GLsizei width = getGlSizeiParameter(args,5);
    GLsizei height = getGlSizeiParameter(args,6);
    GLint border = getGlIntParameter(args,7);
    glCopyTexImage2D(target,level,internalformat,x,y,width,height,border);
}

void copyTexSubImage2D(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLint level = getGlIntParameter(args,1);
    GLint xoffset = getGlIntParameter(args,2);
    GLint yoffset = getGlIntParameter(args,3);
    GLint x = getGlIntParameter(args,4);
    GLint y= getGlIntParameter(args,5);
    GLsizei width = getGlSizeiParameter(args,6);
    GLsizei height = getGlSizeiParameter(args,7);
    glCopyTexSubImage2D(target,level,xoffset,yoffset,x,y,width,height);
}

void createBuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLsizei n = 1;
    GLuint buffer;
    glGenBuffers(n, &buffer);
    if (buffer == 0) {
        args.GetReturnValue().Set(v8::Null(args.GetIsolate()));
    } else {
        args.GetReturnValue().Set(createV8GlObjectFromId(args, buffer));
    }
}

void createProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = glCreateProgram();
    if (program==0) {
        args.GetReturnValue().Set(v8::Null(args.GetIsolate()));
    } else {
        args.GetReturnValue().Set(createV8GlObjectFromId(args,program));
    }
}

void createShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum type = getGlEnumParameter(args,0);
    GLuint shader = glCreateShader(type);
    if (shader==0) {
        args.GetReturnValue().Set(v8::Null(args.GetIsolate()));
    } else {
        args.GetReturnValue().Set(createV8GlObjectFromId(args,shader));
    }
}

void cullFace(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum  mode = getGlEnumParameter(args, 0);
    glCullFace(mode);
}

void deleteRenderbuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLsizei n = 1;
    GLuint buffer = getIdFromV8GlObject(args,0);
    glDeleteRenderbuffers(n,&buffer);
}

void deleteBuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLsizei n = 1;
    GLuint buffer = getIdFromV8GlObject(args,0);
    glDeleteBuffers(n,&buffer);
}

void deleteFramebuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLsizei n = 1;
    GLuint buffer = getIdFromV8GlObject(args,0);
    glDeleteFramebuffers(n,&buffer);
}

void deleteProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    glDeleteProgram(program);
}

void deleteTexture(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLsizei n = 1;
    GLuint texture = getIdFromV8GlObject(args,0);
    glDeleteTextures(n, &texture);
}

void deleteShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint shader = getIdFromV8GlObject(args,0);
    glDeleteShader(shader);
}

void depthFunc(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum func = getGlEnumParameter(args, 0);
    glDepthFunc(func);
}

void depthMask(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLboolean flag = getGlBooleanParameter(args,0);
    glDepthMask(flag);
}

void depthRange(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLfloat n = getGlFloatParameter(args, 0);
    GLfloat f = getGlFloatParameter(args, 1);
    glDepthRangef(n,f);
}

void detachShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    GLuint shader = getIdFromV8GlObject(args,1);
    glDetachShader(program,shader);
}

void disable(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum cap = getGlEnumParameter(args,0);
    glDisable(cap);
}

void disableVertexAttribArray(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint index = getGlUIntParameter(args, 0);
    glDisableVertexAttribArray(index);
}

void drawArrays(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum mode = getGlEnumParameter(args,0);
    GLint first = getGlIntParameter(args,1);
    GLsizei count = getGlSizeiParameter(args,2);
    glDrawArrays(mode,first,count);
}

void drawElements(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum mode = getGlEnumParameter(args,0);
    GLsizei count = getGlSizeiParameter(args,1);
    GLenum type = getGlEnumParameter(args,2);
    void *indices = reinterpret_cast<void *>(getGlUIntParameter(args, 3));
    glDrawElements(mode,count,type,indices);
}

void enable(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum cap = getGlEnumParameter(args,0);
    glEnable(cap);
}

void enableVertexAttribArray(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint index = getGlUIntParameter(args,0);
    glEnableVertexAttribArray(index);
}

void finish(const v8::FunctionCallbackInfo<v8::Value>& args) {
    glFinish();
}

void flush(const v8::FunctionCallbackInfo<v8::Value>& args) {
    glFlush();
}

void framebufferRenderbuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum attachment = getGlEnumParameter(args,1);
    GLenum renderbuffertarget = getGlEnumParameter(args,2);
    GLuint renderbuffer = getIdFromV8GlObject(args,3);
    glFramebufferRenderbuffer(target,attachment,renderbuffertarget,renderbuffer);
}

void framebufferTexture2D(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum attachment = getGlEnumParameter(args,1);
    GLenum textarget = getGlEnumParameter(args,2);
    GLuint texture = getIdFromV8GlObject(args,3);
    GLint level = getGlIntParameter(args,4);
    glFramebufferTexture2D(target,attachment,textarget,texture,level);
}

void frontFace(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum mode = getGlEnumParameter(args,0);
    glFrontFace(mode);
}

void generateMipmap(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    glGenerateMipmap(target);
}

void getActiveAttrib(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    GLuint index = getGlUIntParameter(args,1);
    GLsizei bufSize = 512;
    GLsizei length;
    GLint size;
    GLenum type;
    GLchar name[512];
    glGetActiveAttrib(program,index,bufSize,&length,&size,&type,name);
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    obj->Set(
        isolate->GetCurrentContext(),
        v8::String::NewFromUtf8(isolate, "size").ToLocalChecked(),
        v8::Integer::New(isolate, size)
    ).Check();
    obj->Set(
        isolate->GetCurrentContext(),
        v8::String::NewFromUtf8(isolate, "type").ToLocalChecked(),
        v8::Integer::NewFromUnsigned(isolate, type)
    ).Check();
    obj->Set(
        isolate->GetCurrentContext(),
        v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(),
        v8::String::NewFromUtf8(isolate, name).ToLocalChecked()
    ).Check();
    args.GetReturnValue().Set(obj);
}

void getActiveUniform(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    GLuint index = getGlUIntParameter(args,1);
    GLsizei bufSize = 256;
    GLsizei length;
    GLint size;
    GLenum type;
    GLchar name[256];
    glGetActiveUniform(program,index,bufSize,&length,&size,&type,name);
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    obj->Set(
            isolate->GetCurrentContext(),
            v8::String::NewFromUtf8(isolate, "size").ToLocalChecked(),
            v8::Integer::New(isolate, size)
    ).Check();
    obj->Set(
            isolate->GetCurrentContext(),
            v8::String::NewFromUtf8(isolate, "type").ToLocalChecked(),
            v8::Integer::NewFromUnsigned(isolate, type)
    ).Check();
    obj->Set(
            isolate->GetCurrentContext(),
            v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(),
            v8::String::NewFromUtf8(isolate, name).ToLocalChecked()
    ).Check();
    args.GetReturnValue().Set(obj);
}

void getAttribLocation(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    std::string name = getStringParameter(args,1);
    GLint location = glGetAttribLocation(program,name.c_str());
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),location));
}

void getError(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum error = glGetError();
    args.GetReturnValue().Set(error);
}

void getFramebufferAttachmentParameteriv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum attachment = getGlEnumParameter(args,1);
    GLenum pname = getGlEnumParameter(args,2);
    GLint param;
    glGetFramebufferAttachmentParameteriv(target,attachment,pname,&param);
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),param));
}

void getParameter(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum pname = getGlEnumParameter(args,0);
    GLint data;
    glGetIntegerv(pname,&data);
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),data));
}

void createTexture(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLsizei n = 1;
    GLuint texture;
    glGenTextures(n,&texture);
    args.GetReturnValue().Set(createV8GlObjectFromId(args,texture));
}

void getProgramInfoLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    int length;
    GLchar infoLog[256]; // we can use glGetProgramiv to get the precise length of the string
    glGetProgramInfoLog(program,256,&length,infoLog);
    auto v8Str = v8::String::NewFromUtf8(args.GetIsolate(),infoLog).ToLocalChecked();
    args.GetReturnValue().Set(v8Str);
}

void getProgramParameter(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    GLenum pname = getGlEnumParameter(args,1);
    GLint param;
    glGetProgramiv(program,pname,&param);
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),param));
}

void getRenderbufferParameter(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum pname = getGlEnumParameter(args,1);
    GLint param;
    glGetRenderbufferParameteriv(target,pname,&param);
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),param));
}

void getShaderInfoLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint shader = getIdFromV8GlObject(args,0);
    int length;
    GLchar infoLog[256];// we can use glGetShaderiv to get the precise length of the string
    glGetShaderInfoLog(shader,256,&length,infoLog);
    v8::Local<v8::String> v8Str = v8::String::NewFromUtf8(args.GetIsolate(), infoLog).ToLocalChecked();
    args.GetReturnValue().Set(v8Str);
}

void getShaderPrecisionFormat(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum shadertype = getGlEnumParameter(args,0);
    GLenum precisiontype = getGlEnumParameter(args,1);
    GLint range;
    GLint precision;
    glGetShaderPrecisionFormat(shadertype,precisiontype,&range,&precision);
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    obj->Set(
        isolate->GetCurrentContext(),
        v8::String::NewFromUtf8(isolate, "precision").ToLocalChecked(),
        v8::Integer::New(isolate, precision)
    ).Check();
    obj->Set(
        isolate->GetCurrentContext(),
        v8::String::NewFromUtf8(isolate, "rangeMin").ToLocalChecked(),
        v8::Integer::New(isolate, range)
    ).Check();
    obj->Set(
        isolate->GetCurrentContext(),
        v8::String::NewFromUtf8(isolate, "rangeMax").ToLocalChecked(),
        v8::Integer::New(isolate, range)
    ).Check();
    args.GetReturnValue().Set(obj);
}

void getShaderSource(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint shader = getIdFromV8GlObject(args,0);
//    GLsizei bufsize[1];
//    glGetShaderiv(shader,GL_SHADER_SOURCE_LENGTH,bufsize);
    GLsizei length = 1;
    GLchar source[512];
    glGetShaderSource(shader,512,&length,source);
    auto v8Str = v8::String::NewFromUtf8(args.GetIsolate(),source).ToLocalChecked();
    args.GetReturnValue().Set(v8Str);
}

void getShaderParameter(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint shader = getIdFromV8GlObject(args,0);
    GLenum pname = getGlEnumParameter(args,1);
    GLint param;
    glGetShaderiv(shader,pname,&param);
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),param));
}

void getTexParameter(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum pname = getGlEnumParameter(args,1);
    GLfloat param;
    glGetTexParameterfv(target,pname,&param);
    args.GetReturnValue().Set(v8::Number::New(args.GetIsolate(),param));
}

void getTexParameteri(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum pname = getGlEnumParameter(args,1);
    GLint param;
    glGetTexParameteriv(target,pname,&param);
    args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(),param));
}

void getTexParameterf(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum pname = getGlEnumParameter(args,1);
    GLfloat param;
    glGetTexParameterfv(target,pname,&param);
    args.GetReturnValue().Set(v8::Number::New(args.GetIsolate(),param));
}

void getUniformfv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,1));
    GLfloat param;
    glGetUniformfv(program,location,&param);
    args.GetReturnValue().Set(v8::Number::New(args.GetIsolate(),param));
}

void getUniformLocation(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    std::string name = getStringParameter(args,1);
    GLint location = glGetUniformLocation(program,name.c_str());
    args.GetReturnValue().Set(createV8GlObjectFromId(args,location));
}

void getVertexAttrib(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint index = getGlUIntParameter(args,0);
    GLenum pname = getGlEnumParameter(args,1);
    GLfloat param;
    glGetVertexAttribfv(index,pname,&param);
    args.GetReturnValue().Set(v8::Number::New(args.GetIsolate(),param));
}

void hint(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum mode = getGlEnumParameter(args,1);
    glHint(target,mode);
}

void isBuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint buffer = getIdFromV8GlObject(args,0);
    GLboolean res = glIsBuffer(buffer);
    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(),res==GL_TRUE));
}

void isContextLost(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), false));
}

void isEnabled(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum cap = getGlEnumParameter(args,0);
    GLboolean res = glIsEnabled(cap);
    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(),res==GL_TRUE));
}

void isFramebuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint target = getGlUIntParameter(args,0);
    GLboolean res = glIsFramebuffer(target);
    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(),res==GL_TRUE));
}

void isProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint target = getGlUIntParameter(args,0);
    GLboolean res = glIsProgram(target);
    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(),res==GL_TRUE));
}

void isRenderbuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint target = getGlUIntParameter(args,0);
    GLboolean res = glIsRenderbuffer(target);
    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(),res==GL_TRUE));
}

void isShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint target = getGlUIntParameter(args,0);
    GLboolean res = glIsShader(target);
    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(),res==GL_TRUE));
}

void isTexture(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint target = getGlUIntParameter(args,0);
    GLboolean res = glIsTexture(target);
    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(),res==GL_TRUE));
}

void lineWidth(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLfloat width = getGlFloatParameter(args,0);
    glLineWidth(width);
}

void linkProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    glLinkProgram(program);
}

void pixelStorei(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum pname = getGlIntParameter(args,0);
    if (pname==GL_NONE) return;
    GLint param = getGlIntParameter(args,1);
    glPixelStorei(pname,param);
}

void polygonOffset(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLfloat factor = getGlFloatParameter(args,0);
    GLfloat units = getGlFloatParameter(args,1);
    glPolygonOffset(factor,units);
}

void readPixels(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLint x = getGlIntParameter(args, 0);
    GLint y = getGlIntParameter(args, 1);
    GLsizei width = getGlSizeiParameter(args, 2);
    GLsizei height = getGlSizeiParameter(args, 3);
    GLenum format = getGlEnumParameter(args, 4);
    GLenum type = getGlEnumParameter(args, 5);
    v8::Local<v8::ArrayBuffer> buf_data = getArrayBuffer(args, 6);
    auto backingStore = buf_data->GetBackingStore();
    void* data = backingStore->Data();
    glReadPixels(x, y, width, height, format, type, data);
}

void createRenderbuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLsizei n = 1;
    GLuint buffer;
    glGenRenderbuffers(n,&buffer);
    if (buffer==0) {
        args.GetReturnValue().Set(v8::Null(args.GetIsolate()));
    } else {
        args.GetReturnValue().Set(createV8GlObjectFromId(args,buffer));
    }
}

void createFramebuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLsizei n = 1;
    GLuint buffer;
    glGenFramebuffers(n,&buffer);
    if (buffer==0) {
        args.GetReturnValue().Set(v8::Null(args.GetIsolate()));
    } else {
        args.GetReturnValue().Set(createV8GlObjectFromId(args,buffer));
    }
}

void renderbufferStorage(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum internalformat = getGlEnumParameter(args,1);
    GLsizei width = getGlSizeiParameter(args,2);
    GLsizei height = getGlSizeiParameter(args,3);
    glRenderbufferStorage(target,internalformat,width,height);
}

void sampleCoverage(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLfloat value = getGlFloatParameter(args,0);
    GLboolean invert = getGlBooleanParameter(args,1);
    glSampleCoverage(value,invert);
}

void scissor(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLint x = getGlIntParameter(args,0);
    GLint y = getGlIntParameter(args,1);
    GLsizei width = getGlSizeiParameter(args,2);
    GLsizei height = getGlSizeiParameter(args,3);
    glScissor(x,y,width,height);
}

void shaderSource(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint shader = getIdFromV8GlObject(args,0);
    std::string sourceCstr = getStringParameter(args,1);
    const auto *source = (const GLchar *)sourceCstr.c_str();
    GLsizei count = 1;
    GLint length[1] = {-1};
    glShaderSource(shader,count,&source, length);
}

void stencilFunc(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum func = getGlEnumParameter(args,0);
    GLint ref = getGlIntParameter(args,1);
    GLuint mask = getGlUIntParameter(args,2);
    glStencilFunc(func,ref,mask);
}

void stencilFuncSeparate(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum face = getGlEnumParameter(args,0);
    GLenum func = getGlEnumParameter(args,1);
    GLint ref = getGlIntParameter(args,2);
    GLuint mask = getGlUIntParameter(args,3);
    glStencilFuncSeparate(face,func,ref,mask);
}

void stencilMask(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint mask = getGlUIntParameter(args,0);
    glStencilMask(mask);
}

void stencilMaskSeparate(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum face = getGlEnumParameter(args,0);
    GLuint mask = getGlEnumParameter(args,1);
    glStencilMaskSeparate(face,mask);
}

void stencilOp(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum fail = getGlEnumParameter(args,0);
    GLenum zfail = getGlEnumParameter(args,1);
    GLenum zpass = getGlEnumParameter(args,2);
    glStencilOp(fail,zfail,zpass);
}

void stencilOpSeparate(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum face = getGlEnumParameter(args,0);
    GLenum sfail = getGlEnumParameter(args,1);
    GLenum dpfail = getGlEnumParameter(args,2);
    GLenum dppass = getGlEnumParameter(args,3);
    glStencilOpSeparate(face,sfail,dpfail,dppass);
}

void texParameterf(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum pname = getGlEnumParameter(args,1);
    GLfloat param = getGlFloatParameter(args,2);
    glTexParameterf(target,pname,param);
}

void viewport(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLint x = getGlIntParameter(args,0);
    GLint y = getGlIntParameter(args,1);
    GLsizei width = getGlSizeiParameter(args,2);
    GLsizei height = getGlSizeiParameter(args,3);
    glViewport(x,y,width,height);
}

void validateProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    glValidateProgram(program);
}

void vertexAttribPointer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint index = getGlUIntParameter(args,0);
    GLint size = getGlIntParameter(args,1);
    GLenum type = getGlEnumParameter(args,2);
    GLboolean normalized = getGlBooleanParameter(args,3);
    GLsizei stride = getGlSizeiParameter(args,4);
    void *ptr = reinterpret_cast<void*>(getGlUIntParameter(args, 5));
    glVertexAttribPointer(index,size,type,normalized,stride,ptr);
}

void useProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLuint program = getIdFromV8GlObject(args,0);
    glUseProgram(program);
}

void texParameteri(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLenum pname = getGlEnumParameter(args,1);
    GLint param =  getGlIntParameter(args,2);
    glTexParameteri(target,pname,param);
}

void uniform1f(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLfloat v0 = getGlFloatParameter(args,1);
    glUniform1f(location,v0);
}

void uniform1fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args, 0));
    auto buf_data = getArrayBuffer(args, 1);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4);
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniform1fv(location, size, data);
}

void uniform1i(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLint v0 = getGlIntParameter(args,1);
    glUniform1i(location,v0);
}

void uniform1iv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    auto buf_data = getArrayBuffer(args, 1);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4);
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniform1iv(location,size,(const GLint *)data);
}

void uniform2f(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLfloat v0 = getGlFloatParameter(args,1);
    GLfloat v1 = getGlFloatParameter(args,2);
    glUniform2f(location,v0,v1);
}

void uniform2fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    auto buf_data = getArrayBuffer(args, 1);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4 / 2);
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniform2fv(location,size,(const GLfloat *)data);
}

void uniform2i(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLint v0 = getGlIntParameter(args,1);
    GLint v1 = getGlIntParameter(args,2);
    glUniform2i(location,v0,v1);
}

void uniform2iv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    auto buf_data = getArrayBuffer(args, 1);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4 / 2);
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniform2iv(location,size,(const GLint*)data);
}

void uniform3f(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLfloat v0 = getGlFloatParameter(args,1);
    GLfloat v1 = getGlFloatParameter(args,2);
    GLfloat v2 = getGlFloatParameter(args,3);
    glUniform3f(location,v0,v1,v2);
}

void uniform3fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    auto buf_data = getArrayBuffer(args, 1);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4 / 3);
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniform3fv(location,size,(const GLfloat *)data);
}

void uniform3i(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLint v0 = getGlIntParameter(args,1);
    GLint v1 = getGlIntParameter(args,2);
    GLint v2 = getGlIntParameter(args,3);
    glUniform3i(location,v0,v1,v2);
}

void uniform3iv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    auto buf_data = getArrayBuffer(args, 1);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4 / 3);
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniform3iv(location,size,(const GLint*)data);
}

void uniform4f(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLfloat v0 = getGlFloatParameter(args,1);
    GLfloat v1 = getGlFloatParameter(args,2);
    GLfloat v2 = getGlFloatParameter(args,3);
    GLfloat v3 = getGlFloatParameter(args,4);
    glUniform4f(location,v0,v1,v2,v3);
}

void uniform4fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    auto buf_data = getArrayBuffer(args, 1);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4 / 4);
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniform4fv(location,size,(const GLfloat *)data);
}

void uniform4i(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLint v0 = getGlIntParameter(args,1);
    GLint v1 = getGlIntParameter(args,2);
    GLint v2 = getGlIntParameter(args,3);
    GLint v3 = getGlIntParameter(args,4);
    glUniform4i(location,v0,v1,v2,v3);
}

void uniform4iv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    auto buf_data = getArrayBuffer(args, 1);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4 / 4);
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniform4iv(location,size,(const GLint*)data);
}

void uniformMatrix2fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLboolean transpose = getGlBooleanParameter(args,1);
    auto buf_data = getArrayBuffer(args, 2);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4 / (2*2));
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniformMatrix2fv(location,size,transpose,(const GLfloat *)data);
}

void uniformMatrix3fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLboolean transpose = getGlBooleanParameter(args,1);
    auto buf_data = getArrayBuffer(args, 2);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4 / (3*3));
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniformMatrix3fv(location,size,transpose,(const GLfloat *)data);
}

void uniformMatrix4fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto location = static_cast<GLint>(getIdFromV8GlObject(args,0));
    GLboolean transpose = getGlBooleanParameter(args,1);
    auto buf_data = getArrayBuffer(args, 2);
    auto backingStore = buf_data->GetBackingStore();
    auto size = static_cast<GLsizeiptr>(buf_data->ByteLength() / 4 / (4*4));
    auto* data = static_cast<const GLfloat*>(backingStore->Data());
    glUniformMatrix4fv(location,size,transpose,(const GLfloat *)data);
}


void texImage2D_9(const v8::FunctionCallbackInfo<v8::Value>& args) {
    GLenum target = getGlEnumParameter(args,0);
    GLint level = getGlIntParameter(args,1);
    GLint internalformat = getGlIntParameter(args,2);
    GLsizei width = getGlSizeiParameter(args,3);
    GLsizei height = getGlSizeiParameter(args,4);
    GLint border = getGlIntParameter(args,5);
    GLenum format = getGlEnumParameter(args,6);
    GLenum type = getGlEnumParameter(args,7);

    if (isArrayBuffer(args,8)) {
        auto buf_data = getArrayBuffer(args,8);
        auto backingStore = buf_data->GetBackingStore();
        auto* data = static_cast<const GLfloat*>(backingStore->Data());
        glTexImage2D(target,level,internalformat,width,height,border,format,type,data);
    } else {
        auto *pixels = (void *)calloc(4,width*height);
        glTexImage2D(target,level,internalformat,width,height,border,format,type,pixels);
    }
}

void GlFunctions::create(v8::Isolate *isolate,v8::Local<v8::Context> &context_local, v8::Local<v8::Object> &gl) {

    std::vector<Fun> funcs = {
        {"activeTexture", activeTexture},
        {"attachShader", attachShader},
        {"bindAttribLocation", bindAttribLocation},
        {"bindBuffer", bindBuffer},
        {"bindFramebuffer", bindFramebuffer},
        {"bindRenderbuffer", bindRenderbuffer},
        {"bindTexture", bindTexture},
        {"blendColor", blendColor},
        {"blendEquation", blendEquation},
        {"blendEquationSeparate", blendEquationSeparate},
        {"blendFunc", blendFunc},
        {"blendFuncSeparate", blendFuncSeparate},
        {"bufferData", bufferData},
        {"bufferSubData", bufferSubData},
        {"checkFramebufferStatus", checkFramebufferStatus},
        {"clear", clear},
        {"clearColor", clearColor},
        {"clearStencil", clearStencil},
        {"clearDepth", clearDepth},
        {"colorMask", colorMask},
        {"compileShader", compileShader},
        {"copyTexImage2D", copyTexImage2D},
        {"copyTexSubImage2D", copyTexSubImage2D},
        {"createBuffer", createBuffer},
        {"createProgram", createProgram},
        {"createShader", createShader},
        {"cullFace", cullFace},
        {"deleteRenderbuffer", deleteRenderbuffer},
        {"deleteBuffer", deleteBuffer},
        {"deleteFramebuffer", deleteFramebuffer},
        {"deleteProgram", deleteProgram},
        {"deleteTexture", deleteTexture},
        {"deleteShader", deleteShader},
        {"depthFunc", depthFunc},
        {"depthMask", depthMask},
        {"depthRange", depthRange},
        {"detachShader", detachShader},
        {"disable", disable},
        {"disableVertexAttribArray", disableVertexAttribArray},
        {"drawArrays", drawArrays},
        {"drawElements", drawElements},
        {"enable", enable},
        {"enableVertexAttribArray", enableVertexAttribArray},
        {"finish", finish},
        {"flush", flush},
        {"framebufferRenderbuffer", framebufferRenderbuffer},
        {"framebufferTexture2D", framebufferTexture2D},
        {"frontFace", frontFace},
        {"generateMipmap", generateMipmap},
        {"getActiveAttrib", getActiveAttrib},
        {"getActiveUniform", getActiveUniform},
        {"getAttribLocation", getAttribLocation},
        {"getError", getError},
        {"getFramebufferAttachmentParameteriv", getFramebufferAttachmentParameteriv},
        {"getParameter", getParameter},
        {"createTexture", createTexture},
        {"getProgramInfoLog", getProgramInfoLog},
        {"getProgramParameter", getProgramParameter},
        {"getProgramInfoLog", getProgramInfoLog},
        {"getRenderbufferParameter", getRenderbufferParameter},
        {"getShaderInfoLog", getShaderInfoLog},
        {"getShaderPrecisionFormat", getShaderPrecisionFormat},
        {"getShaderSource", getShaderSource},
        {"getShaderParameter", getShaderParameter},
        {"getTexParameter", getTexParameter},
        {"getTexParameteri", getTexParameteri},
        {"getTexParameterf", getTexParameterf},
        {"getUniformfv", getUniformfv},
        {"getUniformLocation", getUniformLocation},
        {"getVertexAttrib", getVertexAttrib},
        {"hint", hint},
        {"isBuffer", isBuffer},
        {"isContextLost", isContextLost},
        {"isEnabled", isEnabled},
        {"isFramebuffer", isFramebuffer},
        {"isProgram", isProgram},
        {"isRenderbuffer", isRenderbuffer},
        {"isShader", isShader},
        {"isTexture", isTexture},
        {"lineWidth", lineWidth},
        {"linkProgram", linkProgram},
        {"pixelStorei", pixelStorei},
        {"polygonOffset", polygonOffset},
        {"readPixels", readPixels},
        {"createRenderbuffer", createRenderbuffer},
        {"createFramebuffer", createFramebuffer},
        {"renderbufferStorage", renderbufferStorage},
        {"sampleCoverage", sampleCoverage},
        {"scissor", scissor},
        {"stencilFunc", stencilFunc},
        {"stencilFuncSeparate", stencilFuncSeparate},
        {"stencilMask", stencilMask},
        {"stencilMaskSeparate", stencilMaskSeparate},
        {"stencilOp", stencilOp},
        {"stencilOpSeparate", stencilOpSeparate},
        {"texParameterf", texParameterf},
        {"viewport", viewport},
        {"validateProgram", validateProgram},
        {"vertexAttribPointer", vertexAttribPointer},
        {"useProgram", useProgram},
        {"shaderSource", shaderSource},
        {"texParameteri", texParameteri},
        {"uniform1f", uniform1f},
        {"uniform1fv", uniform1fv},
        {"uniform1i", uniform1i},
        {"uniform1iv", uniform1iv},
        {"uniform2f", uniform2f},
        {"uniform2fv", uniform2fv},
        {"uniform2i", uniform2i},
        {"uniform2iv", uniform2iv},
        {"uniform3f", uniform3f},
        {"uniform3fv", uniform3fv},
        {"uniform3i", uniform3i},
        {"uniform3iv", uniform3iv},
        {"uniform4f", uniform4f},
        {"uniform4fv", uniform4fv},
        {"uniform4i", uniform4i},
        {"uniform4iv", uniform4iv},
        {"uniformMatrix2fv", uniformMatrix2fv},
        {"uniformMatrix3fv", uniformMatrix3fv},
        {"uniformMatrix4fv", uniformMatrix4fv},
        {"_texImage2D_9", texImage2D_9},

    };

    for(const Fun& f : funcs) {
        gl->Set(
            context_local,
            v8::String::NewFromUtf8(isolate, f.name.c_str()).ToLocalChecked(),
            v8::Function::New(context_local,f.value).ToLocalChecked()
        ).Check();
    }

}
