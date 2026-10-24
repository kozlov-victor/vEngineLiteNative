
#ifndef VENGINE_EXTERNALFUNCTIONS_H
#define VENGINE_EXTERNALFUNCTIONS_H

#include <v8-isolate.h>

class ExternalFunctions {
public:
    static void create(void* env,v8::Isolate *isolate,v8::Local<v8::Context> &context_local);
};


#endif
