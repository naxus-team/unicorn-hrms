cmake_minimum_required(VERSION 3.20)
project(UnicornDesktop VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# MSVC flags
if(MSVC)
    add_compile_options(/W4 /MP /permissive- /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files
set(SOURCES
    # Core
    src/core/application.cpp
    src/core/window.cpp
    src/core/input.cpp
    src/core/entry_point.cpp
    
    # Renderer
    src/renderer/renderer.cpp
    src/renderer/shader.cpp
    src/renderer/opengl/gl_context.cpp
    src/renderer/opengl/gl_shader.cpp
    src/renderer/opengl/gl_buffer.cpp
    
    # UI
    src/ui/ui_context.cpp
    src/ui/ui_renderer.cpp
    src/ui/font_manager.cpp
    src/ui/text_shaper.cpp
    
    # Database
    src/database/connection.cpp
    
    # Utils
    src/utils/logger.cpp
    
    # Main
    src/application.cpp
    src/main.cpp
    
    # GLAD
    vendor/glad/src/glad.c
)

# Add resource file on Windows
if(WIN32)
    list(APPEND SOURCES src/app.rc)
endif()

add_executable(UnicornDesktop ${SOURCES})

# Include directories
target_include_directories(UnicornDesktop PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/glad/include
    ${CMAKE_SOURCE_DIR}/vendor/glm
    ${CMAKE_SOURCE_DIR}/vendor/stb
    ${CMAKE_SOURCE_DIR}/vendor/freetype/include
    ${CMAKE_SOURCE_DIR}/vendor/harfbuzz/include/harfbuzz
)

# Link libraries
target_link_libraries(UnicornDesktop PRIVATE
    ${CMAKE_SOURCE_DIR}/vendor/glfw/lib/glfw3.lib
    ${CMAKE_SOURCE_DIR}/vendor/freetype/lib/freetype.lib
    ${CMAKE_SOURCE_DIR}/vendor/harfbuzz/lib/harfbuzz.lib
    opengl32
)

# Copy DLLs if needed
if(EXISTS ${CMAKE_SOURCE_DIR}/vendor/freetype/bin/freetype.dll)
    add_custom_command(TARGET UnicornDesktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/vendor/freetype/bin/freetype.dll
        ${CMAKE_SOURCE_DIR}/vendor/harfbuzz/bin/harfbuzz.dll
        ${CMAKE_SOURCE_DIR}/vendor/harfbuzz/bin/harfbuzz-subset.dll
        $<TARGET_FILE_DIR:UnicornDesktop>
    )
endif()

# Copy assets
add_custom_command(TARGET UnicornDesktop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:UnicornDesktop>/assets
)