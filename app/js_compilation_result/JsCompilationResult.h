#ifndef VENGINE_JSCOMPILATIONRESULT_H
#define VENGINE_JSCOMPILATIONRESULT_H
#include <string>

class JsCompilationResult {

public:
    bool success;
    std::string error;

    JsCompilationResult(bool success, std::string error);

};


#endif
