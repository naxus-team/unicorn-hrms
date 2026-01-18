cmake_minimum_required(VERSION 3.20)
project(UnicornDesktop VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

if(MSVC)
    # ========================================
    # MSVC Warning Suppression
    # ========================================
    add_compile_options(
        /W3                     # Warning level 3 (instead of /W4)
        /MP                     # Multi-processor compilation
        /permissive-            # Standards conformance
        /utf-8                  # UTF-8 source and execution charset
        
        # Suppress specific warnings
        /wd4100                 # Unreferenced parameter
        /wd4251                 # DLL interface warnings
        /wd4244                 # Conversion warnings (float to int)
        /wd4005                 # Macro redefinition
        /wd4273                 # Inconsistent DLL linkage
        /wd4217                 # DLL import warnings
    )
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    # GCC/Clang warning suppression
    add_compile_options(
        -Wall
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-sign-compare
    )
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ========================================
# ZLIB Compression Library
# ========================================
set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/vendor/old/zlib)

if(EXISTS ${ZLIB_DIR}/include)
    message(STATUS "ZLIB found at: ${ZLIB_DIR}")
    set(ZLIB_FOUND TRUE)
    set(ZLIB_INCLUDE_DIRS ${ZLIB_DIR}/include)
    if(WIN32)
        set(ZLIB_LIBRARIES ${ZLIB_DIR}/lib/zlib.lib)
    else()
        set(ZLIB_LIBRARIES z)
    endif()
    add_compile_definitions(HAVE_ZLIB)
else()
    message(WARNING "ZLIB not found - Crypto features disabled")
    set(ZLIB_FOUND FALSE)
endif()

# ========================================
# CURL HTTP Library
# ========================================
set(CURL_DIR ${CMAKE_SOURCE_DIR}/vendor/old/curl)

if(EXISTS ${CURL_DIR}/include)
    message(STATUS "CURL found at: ${CURL_DIR}")
    set(CURL_FOUND TRUE)
    set(CURL_INCLUDE_DIRS ${CURL_DIR}/include)
    if(WIN32)
        set(CURL_LIBRARIES ${CURL_DIR}/lib/libcurl.lib)
    else()
        set(CURL_LIBRARIES curl)
    endif()
    add_compile_definitions(HAVE_CURL)
else()
    message(WARNING "CURL not found - Background API disabled")
    set(CURL_FOUND FALSE)
endif()

# ========================================
# Source Files
# ========================================
set(SOURCES
    src/core/application.cpp
    src/core/window.cpp
    src/core/input.cpp
    src/core/entry_point.cpp
    src/renderer/renderer.cpp
    src/renderer/shader.cpp
    src/renderer/opengl/gl_context.cpp
    src/renderer/opengl/gl_shader.cpp
    src/renderer/opengl/gl_buffer.cpp
    src/ui/ui_context.cpp
    src/ui/ui_renderer.cpp
    src/ui/font_manager.cpp
    src/ui/text_shaper.cpp
    src/ui/icon_manager.cpp
    src/ui/ui_animation.cpp
    src/database/connection.cpp
    src/utils/logger.cpp
    src/application.cpp
    src/main.cpp
    vendor/old/glad/src/glad.c
)

if(CURL_FOUND)
    list(APPEND SOURCES src/background/background_manager.cpp)
endif()

if(ZLIB_FOUND)
    list(APPEND SOURCES src/crypto/crypto_manager.cpp)
endif()

if(WIN32)
    list(APPEND SOURCES src/app.rc)
endif()

# ========================================
# Executable
# ========================================
add_executable(UnicornDesktop ${SOURCES})

target_include_directories(UnicornDesktop PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/old/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/old/glad/include
    ${CMAKE_SOURCE_DIR}/vendor/old/glm
    ${CMAKE_SOURCE_DIR}/vendor/old/stb
    ${CMAKE_SOURCE_DIR}/vendor/old/freetype/include
    ${CMAKE_SOURCE_DIR}/vendor/old/harfbuzz/include/harfbuzz
    ${ZLIB_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(UnicornDesktop PRIVATE
    ${CMAKE_SOURCE_DIR}/vendor/old/glfw/lib/glfw3.lib
    ${CMAKE_SOURCE_DIR}/vendor/old/freetype/lib/freetype.lib
    ${CMAKE_SOURCE_DIR}/vendor/old/harfbuzz/lib/harfbuzz.lib
    opengl32
    winmm
    ${ZLIB_LIBRARIES}
    ${CURL_LIBRARIES}
)

if(WIN32 AND CURL_FOUND)
    target_link_libraries(UnicornDesktop PRIVATE wldap32 ws2_32 crypt32 normaliz)
endif()

# ========================================
# Post-Build: Copy DLLs
# ========================================
if(EXISTS ${CMAKE_SOURCE_DIR}/vendor/old/freetype/bin/freetype.dll)
    add_custom_command(TARGET UnicornDesktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/vendor/old/freetype/bin/freetype.dll
        ${CMAKE_SOURCE_DIR}/vendor/old/harfbuzz/bin/harfbuzz.dll
        ${CMAKE_SOURCE_DIR}/vendor/old/harfbuzz/bin/harfbuzz-subset.dll
        $<TARGET_FILE_DIR:UnicornDesktop>
    )
endif()

if(ZLIB_FOUND AND WIN32 AND EXISTS ${ZLIB_DIR}/bin)
    add_custom_command(TARGET UnicornDesktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ZLIB_DIR}/bin/zlib1.dll
        $<TARGET_FILE_DIR:UnicornDesktop>
    )
endif()

if(CURL_FOUND AND WIN32 AND EXISTS ${CURL_DIR}/bin)
    add_custom_command(TARGET UnicornDesktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CURL_DIR}/bin/libcurl.dll
        $<TARGET_FILE_DIR:UnicornDesktop>
    )
endif()

add_custom_command(TARGET UnicornDesktop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:UnicornDesktop>/assets
)

message(STATUS "===========================================")
message(STATUS "Unicorn Desktop Build Configuration")
message(STATUS "===========================================")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "ZLIB: ${ZLIB_FOUND}")
message(STATUS "CURL: ${CURL_FOUND}")
message(STATUS "==========================================")