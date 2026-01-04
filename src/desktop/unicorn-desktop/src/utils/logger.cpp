#include "logger.h"
#include <iostream>

namespace Unicorn {
    void Logger::Info(const std::string& msg) { std::cout << "[INFO] " << msg << std::endl; }
    void Logger::Error(const std::string& msg) { std::cerr << "[ERROR] " << msg << std::endl; }
}
