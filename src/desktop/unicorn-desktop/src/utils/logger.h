#pragma once
#include <string>

namespace Unicorn {
    class Logger {
    public:
        static void Info(const std::string& msg);
        static void Error(const std::string& msg);
    };
}
