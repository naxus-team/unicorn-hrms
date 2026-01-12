#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace Unicorn::UI {

    class IconManager {
    public:
        struct Icon {
            uint32_t textureID;
            int width;
            int height;
        };

        IconManager();
        ~IconManager();

        bool Init();
        void Shutdown();

        // Load icon from SVG file or embedded string
        bool LoadIconFromString(const std::string& name, const std::string& svgContent, int size = 24);

        // Get icon texture
        const Icon* GetIcon(const std::string& name) const;

        // Built-in icons (Material Design style)
        void LoadBuiltInIcons();

        // Clear pre-generated cache (for testing)
        static void ClearCache();

    private:
        std::unordered_map<std::string, Icon> m_Icons;

        // ========================================
        // OPTIMIZED: Pre-generation system
        // ========================================
        void PreGenerateAllIcons();
        void PreRasterizeIcon(const std::string& name, const std::string& svgContent, int size);
        uint32_t CreateTextureFromCache(const std::string& name);

        // Deprecated (kept for compatibility)
        uint32_t RasterizeSVG(const std::string& svgContent, int size);
    };

    // Built-in SVG icons as strings
    namespace Icons {
        // Add icon
        static const char* Add = R"(
            <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                <path d="M22,11.97c0,7.32-2.68,10-10,10s-10-2.68-10-10v0.05c0-7.32,2.68-10,10-10 M22,2.03l-10,9.95" stroke="white" fill="transparent"/>
            </svg>
        )";

        // Settings icon
        static const char* Settings = R"(
            <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                <path d="M12,22.97c-7.89,0-10.98-3.07-11-10.93c0,0,0-0.01,0-0.01c0-0.01,0-0.02,0-0.03s0-0.02,0-0.03c0,0,0-0.01,0-0.01
	C1.02,4.09,4.11,1.03,12,1.03c0.55,0,1,0.45,1,1s-0.45,1-1,1c-6.72,0-8.99,2.26-9,8.97c0.01,6.71,2.28,8.97,9,8.97
	c6.73,0,9-2.27,9-9c0-0.55,0.45-1,1-1s1,0.45,1,1C23,19.89,19.92,22.97,12,22.97z M12,12.97c-0.26,0-0.51-0.1-0.71-0.29
	c-0.39-0.39-0.39-1.02,0-1.41l10-9.95c0.39-0.39,1.02-0.39,1.41,0c0.39,0.39,0.39,1.02,0,1.41l-10,9.95
	C12.51,12.88,12.25,12.97,12,12.97z" fill="white"/>
            </svg>
        )";

        // Close icon
        static const char* Close = R"(
            <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                <path d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z" fill="white"/>
            </svg>
        )";

        // Report icon
        static const char* Report = R"(
            <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                <path d="M19 3H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zM9 17H7v-7h2v7zm4 0h-2V7h2v10zm4 0h-2v-4h2v4z" fill="white"/>
            </svg>
        )";

        // Person icon
        static const char* Person = R"(
            <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                <path d="M12 12c2.21 0 4-1.79 4-4s-1.79-4-4-4-4 1.79-4 4 1.79 4 4 4zm0 2c-2.67 0-8 1.34-8 4v2h16v-2c0-2.66-5.33-4-8-4z" fill="white"/>
            </svg>
        )";
    }

} // namespace Unicorn::UI