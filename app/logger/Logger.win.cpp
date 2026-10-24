
#include <iostream>
#include "Logger.h"

void _log_(const std::string &level, const std::string &message) {
    std::cout << "[" << level << "] " << message << std::endl;
}

void Logger::debug(const std::string &message) {
    _log_("DEBUG", message);
}

void Logger::info(const std::string &message) {
    _log_("INFO", message);
}

void Logger::warn(const std::string &message) {
    _log_("WARN", message);
}

void Logger::error(const std::string &message) {
    _log_("ERROR", message);
}

void Logger::debug(const std::string &message, int val) {
    debug(message + " " + std::to_string(val));
}

void Logger::info(const std::string &message, int val) {
    info(message + " " + std::to_string(val));
}

void Logger::warn(const std::string &message, int val) {
    warn(message + " " + std::to_string(val));
}

void Logger::error(const std::string &message, int val) {
    error(message + " " + std::to_string(val));
}