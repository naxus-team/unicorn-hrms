#include "gl_context.h"
#include <glad/glad.h>

namespace Unicorn {
    bool GLContext::Init() {
        return gladLoadGL() != 0;
    }
}
