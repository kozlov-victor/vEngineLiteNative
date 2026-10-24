#ifndef VENGINE_CONSOLE_H
#define VENGINE_CONSOLE_H


#include <v8-isolate.h>

class Console {

public:
    static void create(v8::Isolate *isolate,v8::Local<v8::Context> &context);
};


#endif
