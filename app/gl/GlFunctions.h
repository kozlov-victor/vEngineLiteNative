
#ifndef VENGINE_GLFUNCTIONS_H
#define VENGINE_GLFUNCTIONS_H


#include <v8-isolate.h>

class GlFunctions {
public:
    static void create(v8::Isolate *isolate,v8::Local<v8::Context> &context_local,v8::Local<v8::Object> &gl);
};


#endif
