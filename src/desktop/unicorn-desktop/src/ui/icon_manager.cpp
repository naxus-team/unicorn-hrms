#include "icon_manager.h"
#include <glad/glad.h>
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#include <iostream>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

namespace Unicorn::UI {

    IconManager::IconManager() {}

    IconManager::~IconManager() {
        Shutdown();
    }

    bool IconManager::Init() {
        LoadBuiltInIcons();
        std::cout << "[IconManager] Initialized with " << m_Icons.size() << " icons" << std::endl;
        return true;
    }

    void IconManager::Shutdown() {
        for (auto& [name, icon] : m_Icons) {
            if (icon.textureID) {
                glDeleteTextures(1, &icon.textureID);
            }
        }
        m_Icons.clear();
    }

    uint32_t IconManager::RasterizeSVG(const std::string& svgContent, int size) {
        // Parse SVG
        NSVGimage* image = nsvgParse((char*)svgContent.c_str(), "px", 96.0f);
        if (!image) {
            std::cerr << "[IconManager] Failed to parse SVG" << std::endl;
            return 0;
        }

        int supersampleFactor = 8;
        int renderSize = size * supersampleFactor;

        // Calculate scale
        float scale = (float)renderSize / std::max(image->width, image->height);

        int channels = 4; // RGBA
        unsigned char* pixels = new unsigned char[renderSize * renderSize * channels];
        memset(pixels, 0, renderSize * renderSize * channels);

        NSVGrasterizer* rast = nsvgCreateRasterizer();

        nsvgRasterize(rast, image, 0, 0, scale, pixels, renderSize, renderSize, renderSize * channels);

        for (int i = 0; i < renderSize * renderSize; i++) {
            int idx = i * 4;
            float alpha = pixels[idx + 3] / 255.0f;
            pixels[idx + 0] = (unsigned char)(pixels[idx + 0] * alpha); // R
            pixels[idx + 1] = (unsigned char)(pixels[idx + 1] * alpha); // G
            pixels[idx + 2] = (unsigned char)(pixels[idx + 2] * alpha); // B
        }

        uint32_t texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderSize, renderSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        float maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, glm::min(maxAnisotropy, 16.0f));

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.5f);

        // Cleanup
        delete[] pixels;
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);

        std::cout << "[IconManager] Rasterized icon at " << renderSize << "px (8x supersampling)" << std::endl;

        return texture;
    }

    bool IconManager::LoadIconFromString(const std::string& name, const std::string& svgContent, int size) {
        uint32_t texture = RasterizeSVG(svgContent, size);
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
        LoadIconFromString("add", Icons::Add, 20);
        LoadIconFromString("settings", Icons::Settings, 20);
        LoadIconFromString("close", Icons::Close, 20);
        LoadIconFromString("report", Icons::Report, 20);
        LoadIconFromString("person", Icons::Person, 20);
    }

} // namespace Unicorn::UI