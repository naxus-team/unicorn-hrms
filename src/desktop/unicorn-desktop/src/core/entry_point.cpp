#include "entry_point.h"
#include "application.h"
#include <iostream>

namespace Unicorn {
    int EntryPoint(int argc, char** argv) {
        try {
            auto app = CreateApplication();
            app->Run();
            delete app;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        return 0;
    }
}