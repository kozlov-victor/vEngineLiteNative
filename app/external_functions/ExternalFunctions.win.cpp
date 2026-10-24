#include <v8-function.h>
#include <v8-context.h>


#include "ExternalFunctions.h"
#include "../logger/Logger.h"
#include "../misc/fun.hpp"

// MessageBoxW(
//     nullptr,
//     L"Помилка JavaScript. Дивись vengine.log біля EXE.",
//     L"vEngineLite Native",
//     MB_OK | MB_ICONERROR
// );

void ExternalFunctions::create(void* env, v8::Isolate *isolate, v8::Local<v8::Context> &context_local) {

}
