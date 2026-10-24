#ifndef VENGINE_LOGGER_H
#define VENGINE_LOGGER_H


#include <string>

class Logger {

public:

    void static debug(const std::string &message);
    void static info(const std::string &message);
    void static warn(const std::string &message);
    void static error(const std::string &message);

    void static debug(const std::string &message, int val);
    void static info(const std::string &message, int val);
    void static warn(const std::string &message, int val);
    void static error(const std::string &message, int val);

};


#endif
