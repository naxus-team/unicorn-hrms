#pragma once

#include "text_shaper.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <cstdint>

// Forward declare FreeType types
typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;

namespace Unicorn::UI {

    struct Character {
        uint32_t textureID;  // OpenGL texture ID
        glm::ivec2 size;     // Size of glyph
        glm::ivec2 bearing;  // Offset from baseline to left/top of glyph
        uint32_t advance;    // Offset to advance to next glyph
    };

    // ============================================
    // NEW: Font rendering options
    // ============================================
    struct FontRenderOptions {
        bool useKerning = true;           // Enable kerning pairs
        bool useHinting = true;           // FreeType hinting
        bool useAntialiasing = true;      // Enable AA
        float letterSpacing = 0.0f;       // Extra spacing between letters (pixels)
        float lineHeight = 1.2f;          // Line height multiplier

        enum class AntialiasMode {
            None,       // No antialiasing (faster)
            Grayscale,  // Standard grayscale AA (default)
            LCD         // LCD subpixel rendering (best quality)
        };
        AntialiasMode aaMode = AntialiasMode::Grayscale;
    };

    class FontManager {
    public:
        FontManager();
        ~FontManager();

        bool Init();
        void Shutdown();

        // ============================================
        // ORIGINAL METHODS - Keep compatibility
        // ============================================
        bool LoadFont(const std::string& name, const std::string& filepath, uint32_t fontSize);
        bool SetActiveFont(const std::string& name);
        bool LoadUnicodeRange(const std::string& fontName, uint32_t start, uint32_t end);

        const Character& GetCharacter(uint32_t codepoint) const;
        const Character& GetCharacterByGlyphIndex(uint32_t glyphIndex);

        glm::vec2 CalculateTextSize(const std::string& utf8Text, float scale = 1.0f) const;
        std::vector<ShapedGlyph> ShapeText(const std::string& utf8Text);

        uint32_t GetFontTexture() const { return m_FontAtlasTexture; }
        const std::unordered_map<uint32_t, Character>& GetCharacters() const { return m_ActiveCharacters; }
        TextShaper& GetTextShaper() { return *m_TextShaper; }

        // ============================================
        // NEW METHODS - Enhanced features
        // ============================================

        // Load font with custom rendering options
        bool LoadFontWithOptions(const std::string& name, const std::string& filepath,
            uint32_t fontSize, const FontRenderOptions& options);

        // Kerning support
        float GetKerning(uint32_t leftCodepoint, uint32_t rightCodepoint) const;
        void EnableKerning(bool enable) { m_RenderOptions.useKerning = enable; }
        bool IsKerningEnabled() const { return m_RenderOptions.useKerning; }

        // Render options access
        FontRenderOptions& GetRenderOptions() { return m_RenderOptions; }
        const FontRenderOptions& GetRenderOptions() const { return m_RenderOptions; }
        void SetRenderOptions(const FontRenderOptions& options);

        // Get active font name
        const std::string& GetActiveFontName() const { return m_ActiveFontName; }

    private:
        bool LoadCharacters(FT_Face face, uint32_t fontSize);
        bool LoadCharacterRange(FT_Face face, uint32_t start, uint32_t end);
        bool LoadGlyphByIndex(FT_Face face, uint32_t glyphIndex);

        // NEW: Cache kerning pairs
        void CacheKerning(FT_Face face, uint32_t left, uint32_t right);

        // NEW: Enhanced texture generation with AA control
        uint32_t GenerateCharacterTexture(FT_Face face, uint32_t codepoint);

        // UTF-8 helper functions
        static uint32_t UTF8ToCodepoint(const char*& str);
        static size_t UTF8CharLength(const char* str);

    private:
        FT_Library m_FTLibrary = nullptr;

        struct FontData {
            std::unordered_map<uint32_t, Character> characters;
            std::unordered_map<uint32_t, Character> glyphCache;
            std::unordered_map<uint64_t, float> kerningCache;  // NEW: (left << 32 | right) -> kerning
            uint32_t fontSize;
            FT_Face face;
            FontRenderOptions renderOptions;  // NEW: Per-font options
        };

        std::unordered_map<std::string, FontData> m_Fonts;
        std::unordered_map<uint32_t, Character> m_ActiveCharacters;
        std::unordered_map<uint32_t, Character> m_ActiveGlyphCache;
        std::unordered_map<uint64_t, float> m_ActiveKerningCache;  // NEW

        std::string m_ActiveFontName;
        FT_Face m_ActiveFace = nullptr;
        uint32_t m_FontAtlasTexture = 0;
        Character m_DefaultCharacter;

        std::unique_ptr<TextShaper> m_TextShaper;
        FontRenderOptions m_RenderOptions;  // NEW: Active render options
    };

} // namespace Unicorn::UI