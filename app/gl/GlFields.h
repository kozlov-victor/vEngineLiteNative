
#ifndef VENGINE_GLFIELDS_H
#define VENGINE_GLFIELDS_H


#include <v8-isolate.h>

class GlFields {
public:
    static void create(v8::Isolate *isolate,v8::Local<v8::Context> &context_local,v8::Local<v8::Object> &gl);
};


#endif
