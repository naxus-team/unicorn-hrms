#pragma once
#include <string>
#include <unordered_map>
#include <memory>
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
        bool LoadIconFromFile(const std::string& name, const std::string& filepath, int size = 24);
        bool LoadIconFromString(const std::string& name, const std::string& svgContent, int size = 24);

        // Get icon texture
        const Icon* GetIcon(const std::string& name) const;

        // Built-in icons (Material Design style)
        void LoadBuiltInIcons();

    private:
        std::unordered_map<std::string, Icon> m_Icons;

        uint32_t RasterizeSVG(const std::string& svgContent, int size);
    };

    // Built-in SVG icons as strings
    namespace Icons {
        // Add icon
        static const char* Add = R"(
            <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                <path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z" fill="white"/>
            </svg>
        )";

        // Settings icon
        static const char* Settings = R"(
            <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                <path d="M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58c.18-.14.23-.41.12-.61l-1.92-3.32c-.12-.22-.37-.29-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94L14.4 2.81c-.04-.24-.24-.41-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96c-.22-.08-.47 0-.59.22L2.74 8.87c-.12.21-.08.47.12.61l2.03 1.58c-.05.3-.09.63-.09.94s.02.64.07.94l-2.03 1.58c-.18.14-.23.41-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62 3.6 3.6-1.62 3.6-3.6 3.6z" fill="white"/>
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