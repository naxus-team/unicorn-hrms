#include "icon_manager.h"
#include <glad/glad.h>
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#include <iostream>
#include <chrono>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

namespace Unicorn::UI {

    // ========================================
    // CRITICAL: Icon cache to avoid re-rasterization
    // ========================================
    struct CachedIconData {
        std::vector<unsigned char> pixels;
        int width;
        int height;
        bool generated;
    };

    static std::unordered_map<std::string, CachedIconData> s_IconCache;
    static bool s_IconsPreGenerated = false;

    IconManager::IconManager() {}

    IconManager::~IconManager() {
        Shutdown();
    }

    bool IconManager::Init() {
        auto startTime = std::chrono::high_resolution_clock::now();

        // ========================================
        // Pre-generate all icons ONCE at startup
        // ========================================
        if (!s_IconsPreGenerated) {
            std::cout << "[IconManager] Pre-generating icon textures..." << std::endl;
            PreGenerateAllIcons();
            s_IconsPreGenerated = true;
        }

        // Load from cache (instant)
        LoadBuiltInIcons();

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << "[IconManager] Initialized with " << m_Icons.size()
            << " icons in " << duration.count() << "ms" << std::endl;
        return true;
    }

    void IconManager::Shutdown() {
        for (auto& [name, icon] : m_Icons) {
            if (icon.textureID) {
                glDeleteTextures(1, &icon.textureID);
            }
        }
        m_Icons.clear();

        // Keep s_IconCache alive for fast reload
        std::cout << "[IconManager] Shutdown (cache retained for fast reload)" << std::endl;
    }

    void IconManager::PreGenerateAllIcons() {
        // Pre-generate all built-in icons at high quality
        const int iconSize = 20;

        struct IconDef {
            const char* name;
            const char* svg;
        };

        IconDef icons[] = {
            {"add", Icons::Add},
            {"settings", Icons::Settings},
            {"close", Icons::Close},
            {"report", Icons::Report},
            {"person", Icons::Person}
        };

        for (const auto& iconDef : icons) {
            if (s_IconCache.find(iconDef.name) == s_IconCache.end()) {
                std::cout << "[IconManager]   Pre-rasterizing: " << iconDef.name << std::endl;
                PreRasterizeIcon(iconDef.name, iconDef.svg, iconSize);
            }
        }
    }

    void IconManager::PreRasterizeIcon(const std::string& name, const std::string& svgContent, int size) {
        // Parse SVG
        NSVGimage* image = nsvgParse((char*)svgContent.c_str(), "px", 96.0f);
        if (!image) {
            std::cerr << "[IconManager] Failed to parse SVG: " << name << std::endl;
            return;
        }

        // 8x supersampling for ultra-crisp icons
        int supersampleFactor = 8;
        int renderSize = size * supersampleFactor;

        // Calculate scale
        float scale = (float)renderSize / std::max(image->width, image->height);

        int channels = 4; // RGBA
        std::vector<unsigned char> pixels(renderSize * renderSize * channels, 0);

        NSVGrasterizer* rast = nsvgCreateRasterizer();

        nsvgRasterize(rast, image, 0, 0, scale, pixels.data(),
            renderSize, renderSize, renderSize * channels);

        // Premultiply alpha for correct blending
        for (int i = 0; i < renderSize * renderSize; i++) {
            int idx = i * 4;
            float alpha = pixels[idx + 3] / 255.0f;
            pixels[idx + 0] = (unsigned char)(pixels[idx + 0] * alpha); // R
            pixels[idx + 1] = (unsigned char)(pixels[idx + 1] * alpha); // G
            pixels[idx + 2] = (unsigned char)(pixels[idx + 2] * alpha); // B
        }

        // Store in cache
        CachedIconData cached;
        cached.pixels = std::move(pixels);
        cached.width = renderSize;
        cached.height = renderSize;
        cached.generated = true;
        s_IconCache[name] = std::move(cached);

        // Cleanup
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);
    }

    uint32_t IconManager::RasterizeSVG(const std::string& svgContent, int size) {
        // ❌ OLD: This was called every Init()
        // Now we use cached data instead!
        return 0; // Deprecated
    }

    uint32_t IconManager::CreateTextureFromCache(const std::string& name) {
        auto it = s_IconCache.find(name);
        if (it == s_IconCache.end() || !it->second.generated) {
            std::cerr << "[IconManager] Icon not in cache: " << name << std::endl;
            return 0;
        }

        const auto& cached = it->second;

        // Create OpenGL texture from cached pixel data
        uint32_t texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
            cached.width, cached.height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, cached.pixels.data());

        // Generate mipmaps for smooth scaling
        glGenerateMipmap(GL_TEXTURE_2D);

        // High-quality filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Anisotropic filtering (if available)
        float maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
            glm::min(maxAnisotropy, 16.0f));

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Sharpen mipmap transitions
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.5f);

        glBindTexture(GL_TEXTURE_2D, 0);

        return texture;
    }

    bool IconManager::LoadIconFromString(const std::string& name, const std::string& svgContent, int size) {
        // Check if already in cache
        if (s_IconCache.find(name) == s_IconCache.end()) {
            // Not cached - generate now (only happens for dynamic icons)
            PreRasterizeIcon(name, svgContent, size);
        }

        // Create texture from cache
        uint32_t texture = CreateTextureFromCache(name);
        if (texture == 0) {
            return false;
        }

        Icon icon;
        icon.textureID = texture;
        icon.width = size;
        icon.height = size;
        m_Icons[name] = icon;

        return true;
    }

    const IconManager::Icon* IconManager::GetIcon(const std::string& name) const {
        auto it = m_Icons.find(name);
        if (it != m_Icons.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void IconManager::LoadBuiltInIcons() {
        // ✅ Load from pre-generated cache (instant!)
        LoadIconFromString("add", Icons::Add, 20);
        LoadIconFromString("settings", Icons::Settings, 20);
        LoadIconFromString("close", Icons::Close, 20);
        LoadIconFromString("report", Icons::Report, 20);
        LoadIconFromString("person", Icons::Person, 20);
    }

    void IconManager::ClearCache() {
        s_IconCache.clear();
        s_IconsPreGenerated = false;
        std::cout << "[IconManager] Cache cleared" << std::endl;
    }

} // namespace Unicorn::UI