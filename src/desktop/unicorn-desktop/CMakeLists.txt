cmake_minimum_required(VERSION 3.20)
project(UnicornDesktop VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

if(MSVC)
    add_compile_options(/W4 /MP /permissive- /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ========================================
# ZLIB Compression Library
# ========================================
set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/vendor/zlib)

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
    message(WARNING "ZLIB not found at: ${ZLIB_DIR}")
    message(WARNING "Crypto/Compression features will be disabled")
    set(ZLIB_FOUND FALSE)
    set(ZLIB_INCLUDE_DIRS "")
    set(ZLIB_LIBRARIES "")
endif()

# ========================================
# CURL HTTP Library
# ========================================
set(CURL_DIR ${CMAKE_SOURCE_DIR}/vendor/curl)

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
    message(WARNING "CURL not found at: ${CURL_DIR}")
    message(WARNING "Background API Manager will be disabled")
    set(CURL_FOUND FALSE)
    set(CURL_INCLUDE_DIRS "")
    set(CURL_LIBRARIES "")
endif()

# ========================================
# FFmpeg Video Library (Optional)
# ========================================
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/vendor/ffmpeg)

if(EXISTS ${FFMPEG_DIR}/include)
    message(STATUS "FFmpeg found at: ${FFMPEG_DIR}")
    set(FFMPEG_FOUND TRUE)
    
    set(FFMPEG_INCLUDE_DIRS ${FFMPEG_DIR}/include)
    
    if(WIN32)
        set(FFMPEG_LIBRARIES
            ${FFMPEG_DIR}/lib/avformat.lib
            ${FFMPEG_DIR}/lib/avcodec.lib
            ${FFMPEG_DIR}/lib/avutil.lib
            ${FFMPEG_DIR}/lib/swscale.lib
            ${FFMPEG_DIR}/lib/swresample.lib
        )
    else()
        set(FFMPEG_LIBRARIES
            avformat avcodec avutil swscale swresample
        )
    endif()
    
    add_compile_definitions(HAVE_FFMPEG)
else()
    message(WARNING "FFmpeg not found at: ${FFMPEG_DIR}")
    message(WARNING "Video player will be disabled")
    set(FFMPEG_FOUND FALSE)
    set(FFMPEG_INCLUDE_DIRS "")
    set(FFMPEG_LIBRARIES "")
endif()

# ========================================
# SoLoud Audio Library
# ========================================
set(SOLOUD_DIR ${CMAKE_SOURCE_DIR}/vendor/soloud)

set(SOLOUD_SOURCES
    ${SOLOUD_DIR}/src/core/soloud.cpp
    ${SOLOUD_DIR}/src/core/soloud_audiosource.cpp
    ${SOLOUD_DIR}/src/core/soloud_bus.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_3d.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_basicops.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_faderops.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_filterops.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_getters.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_setters.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_voicegroup.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_voiceops.cpp
    ${SOLOUD_DIR}/src/core/soloud_fader.cpp
    ${SOLOUD_DIR}/src/core/soloud_fft.cpp
    ${SOLOUD_DIR}/src/core/soloud_fft_lut.cpp
    ${SOLOUD_DIR}/src/core/soloud_file.cpp
    ${SOLOUD_DIR}/src/core/soloud_filter.cpp
    ${SOLOUD_DIR}/src/core/soloud_misc.cpp
    ${SOLOUD_DIR}/src/core/soloud_queue.cpp
    ${SOLOUD_DIR}/src/core/soloud_thread.cpp
    ${SOLOUD_DIR}/src/audiosource/wav/soloud_wav.cpp
    ${SOLOUD_DIR}/src/audiosource/wav/soloud_wavstream.cpp
    ${SOLOUD_DIR}/src/audiosource/wav/stb_vorbis.c
    ${SOLOUD_DIR}/src/audiosource/wav/dr_impl.cpp
    ${SOLOUD_DIR}/src/filter/soloud_bassboostfilter.cpp
    ${SOLOUD_DIR}/src/filter/soloud_biquadresonantfilter.cpp
    ${SOLOUD_DIR}/src/filter/soloud_dcremovalfilter.cpp
    ${SOLOUD_DIR}/src/filter/soloud_echofilter.cpp
    ${SOLOUD_DIR}/src/filter/soloud_fftfilter.cpp
    ${SOLOUD_DIR}/src/filter/soloud_flangerfilter.cpp
    ${SOLOUD_DIR}/src/filter/soloud_freeverbfilter.cpp
    ${SOLOUD_DIR}/src/filter/soloud_lofifilter.cpp
    ${SOLOUD_DIR}/src/filter/soloud_robotizefilter.cpp
    ${SOLOUD_DIR}/src/filter/soloud_waveshaperfilter.cpp
)

if(WIN32)
    list(APPEND SOLOUD_SOURCES ${SOLOUD_DIR}/src/backend/winmm/soloud_winmm.cpp)
endif()

add_library(soloud STATIC ${SOLOUD_SOURCES})
target_include_directories(soloud PUBLIC ${SOLOUD_DIR}/include)

if(WIN32)
    target_compile_definitions(soloud PRIVATE WITH_WINMM)
    target_link_libraries(soloud PRIVATE winmm)
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
    src/audio/sound_manager.cpp
    src/database/connection.cpp
    src/utils/logger.cpp
    src/application.cpp
    src/main.cpp
    vendor/glad/src/glad.c
)

if(CURL_FOUND)
    list(APPEND SOURCES src/background/background_manager.cpp)
    message(STATUS "Background API Manager enabled")
else()
    message(STATUS "Background API Manager disabled (CURL not found)")
endif()

if(FFMPEG_FOUND)
    list(APPEND SOURCES src/video/video_player.cpp)
    message(STATUS "Video player enabled")
else()
    message(STATUS "Video player disabled (FFmpeg not found)")
endif()

if(ZLIB_FOUND)
    list(APPEND SOURCES src/crypto/crypto_manager.cpp)
    message(STATUS "Crypto/Compression Manager enabled")
else()
    message(STATUS "Crypto/Compression Manager disabled (ZLIB not found)")
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
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/glad/include
    ${CMAKE_SOURCE_DIR}/vendor/glm
    ${CMAKE_SOURCE_DIR}/vendor/stb
    ${CMAKE_SOURCE_DIR}/vendor/freetype/include
    ${CMAKE_SOURCE_DIR}/vendor/harfbuzz/include/harfbuzz
    ${CMAKE_SOURCE_DIR}/vendor/soloud/include
    ${ZLIB_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(UnicornDesktop PRIVATE
    ${CMAKE_SOURCE_DIR}/vendor/glfw/lib/glfw3.lib
    ${CMAKE_SOURCE_DIR}/vendor/freetype/lib/freetype.lib
    ${CMAKE_SOURCE_DIR}/vendor/harfbuzz/lib/harfbuzz.lib
    opengl32
    soloud
    winmm
    ${ZLIB_LIBRARIES}
    ${CURL_LIBRARIES}
    ${FFMPEG_LIBRARIES}
)

if(WIN32 AND CURL_FOUND)
    target_link_libraries(UnicornDesktop PRIVATE wldap32 ws2_32 crypt32 normaliz)
endif()

# ========================================
# Post-Build: Copy DLLs
# ========================================

if(EXISTS ${CMAKE_SOURCE_DIR}/vendor/freetype/bin/freetype.dll)
    add_custom_command(TARGET UnicornDesktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/vendor/freetype/bin/freetype.dll
        ${CMAKE_SOURCE_DIR}/vendor/harfbuzz/bin/harfbuzz.dll
        ${CMAKE_SOURCE_DIR}/vendor/harfbuzz/bin/harfbuzz-subset.dll
        $<TARGET_FILE_DIR:UnicornDesktop>
    )
endif()

if(ZLIB_FOUND AND WIN32 AND EXISTS ${ZLIB_DIR}/bin)
    add_custom_command(TARGET UnicornDesktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ZLIB_DIR}/bin/zlib1.dll
        $<TARGET_FILE_DIR:UnicornDesktop>
        COMMENT "Copying ZLIB DLL..."
    )
endif()

if(CURL_FOUND AND WIN32 AND EXISTS ${CURL_DIR}/bin)
    add_custom_command(TARGET UnicornDesktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CURL_DIR}/bin/libcurl.dll
        $<TARGET_FILE_DIR:UnicornDesktop>
        COMMENT "Copying CURL DLL..."
    )
endif()

if(FFMPEG_FOUND AND WIN32 AND EXISTS ${FFMPEG_DIR}/bin)
    add_custom_command(TARGET UnicornDesktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FFMPEG_DIR}/bin/avformat-62.dll
        ${FFMPEG_DIR}/bin/avcodec-62.dll
        ${FFMPEG_DIR}/bin/avutil-60.dll
        ${FFMPEG_DIR}/bin/swscale-9.dll
        ${FFMPEG_DIR}/bin/swresample-6.dll
        ${FFMPEG_DIR}/bin/avfilter-11.dll
        ${FFMPEG_DIR}/bin/avdevice-62.dll
        $<TARGET_FILE_DIR:UnicornDesktop>
        COMMENT "Copying FFmpeg DLLs..."
    )
endif()

add_custom_command(TARGET UnicornDesktop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:UnicornDesktop>/assets
    COMMENT "Copying assets..."
)

# ========================================
# Build Summary
# ========================================
message(STATUS "===========================================")
message(STATUS "Unicorn Desktop Build Configuration")
message(STATUS "===========================================")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "ZLIB: ${ZLIB_FOUND}")
message(STATUS "CURL: ${CURL_FOUND}")
message(STATUS "FFmpeg: ${FFMPEG_FOUND}")
message(STATUS "Crypto/Compression Manager: ${ZLIB_FOUND}")
message(STATUS "Background API Manager: ${CURL_FOUND}")
message(STATUS "Video Player: ${FFMPEG_FOUND}")
message(STATUS "===========================================")