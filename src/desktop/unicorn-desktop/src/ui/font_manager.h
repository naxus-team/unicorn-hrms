#pragma once

#include "text_shaper.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <cstdint>
#include <fstream>

typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;

namespace Unicorn::UI {

    struct Character {
        uint32_t textureID;     // Atlas texture ID (shared) or individual texture
        glm::vec2 atlasPos;     // Position in atlas (normalized 0-1)
        glm::vec2 atlasSize;    // Size in atlas (normalized 0-1)
        glm::ivec2 size;        // Size of glyph in pixels
        glm::ivec2 bearing;     // Offset from baseline
        uint32_t advance;       // Horizontal advance
    };

    struct FontRenderOptions {
        bool useKerning = true;
        bool useHinting = true;
        bool useAntialiasing = true;
        float letterSpacing = 0.0f;
        float lineHeight = 1.0f;
        float weight = 0.0f;
        float baselineOffset = 0.0f;

        enum class AntialiasMode {
            None,
            Grayscale,
            LCD
        };
        AntialiasMode aaMode = AntialiasMode::Grayscale;
    };

    // Font Atlas for efficient GPU memory usage
    struct FontAtlas {
        uint32_t textureID = 0;
        uint32_t width = 2048;      // Atlas texture width
        uint32_t height = 2048;     // Atlas texture height
        uint32_t currentX = 0;      // Current packing position X
        uint32_t currentY = 0;      // Current packing position Y
        uint32_t rowHeight = 0;     // Current row height

        bool AddGlyph(uint32_t glyphWidth, uint32_t glyphHeight,
            const unsigned char* pixelData,
            glm::vec2& outAtlasPos, glm::vec2& outAtlasSize);
        void Clear();
    };

    class FontManager {
    public:
        FontManager();
        ~FontManager();

        bool Init();
        void Shutdown();

        bool LoadFont(const std::string& name, const std::string& filepath, uint32_t fontSize);
        bool LoadFontWithOptions(const std::string& name, const std::string& filepath,
            uint32_t fontSize, const FontRenderOptions& options);
        bool SetActiveFont(const std::string& name);
        bool LoadUnicodeRange(const std::string& fontName, uint32_t start, uint32_t end);

        const Character& GetCharacter(uint32_t codepoint) const;
        const Character& GetCharacterByGlyphIndex(uint32_t glyphIndex);

        glm::vec2 CalculateTextSize(const std::string& utf8Text, float scale = 1.0f) const;
        std::vector<ShapedGlyph> ShapeText(const std::string& utf8Text);

        uint32_t GetFontAtlasTexture() const { return m_Atlas.textureID; }
        const std::unordered_map<uint32_t, Character>& GetCharacters() const { return m_ActiveCharacters; }
        TextShaper& GetTextShaper() { return *m_TextShaper; }

        float GetKerning(uint32_t leftCodepoint, uint32_t rightCodepoint) const;
        void EnableKerning(bool enable) { m_RenderOptions.useKerning = enable; }
        bool IsKerningEnabled() const { return m_RenderOptions.useKerning; }

        FontRenderOptions& GetRenderOptions() { return m_RenderOptions; }
        const FontRenderOptions& GetRenderOptions() const { return m_RenderOptions; }
        void SetRenderOptions(const FontRenderOptions& options);

        FT_Face GetActiveFace() const { return m_ActiveFace; }
        const std::string& GetActiveFontName() const { return m_ActiveFontName; }

    private:
        bool LoadCharacters(FT_Face face, uint32_t fontSize);
        bool LoadCharacterRange(FT_Face face, uint32_t start, uint32_t end);
        bool LoadGlyphByIndex(FT_Face face, uint32_t glyphIndex);
        void CacheKerning(FT_Face face, uint32_t left, uint32_t right);
        uint32_t GenerateCharacterTexture(FT_Face face, uint32_t codepoint);

        static uint32_t UTF8ToCodepoint(const char*& str);
        static size_t UTF8CharLength(const char* str);

        FT_Library m_FTLibrary = nullptr;

        struct FontData {
            std::unordered_map<uint32_t, Character> characters;
            std::unordered_map<uint32_t, Character> glyphCache;
            std::unordered_map<uint64_t, float> kerningCache;
            uint32_t fontSize;
            FT_Face face;
            FontRenderOptions renderOptions;
        };

        std::unordered_map<std::string, FontData> m_Fonts;
        std::unordered_map<uint32_t, Character> m_ActiveCharacters;
        std::unordered_map<uint32_t, Character> m_ActiveGlyphCache;
        std::unordered_map<uint64_t, float> m_ActiveKerningCache;

        std::string m_ActiveFontName;
        FT_Face m_ActiveFace = nullptr;
        Character m_DefaultCharacter;

        std::unique_ptr<TextShaper> m_TextShaper;
        FontRenderOptions m_RenderOptions;

        // Font Atlas
        FontAtlas m_Atlas;
    };

} // namespace Unicorn::UI