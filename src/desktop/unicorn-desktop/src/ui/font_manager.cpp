#include "font_manager.h"
#include <glad/glad.h>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Unicorn::UI {

    FontManager::FontManager() {
        // Initialize default character
        m_DefaultCharacter.textureID = 0;
        m_DefaultCharacter.size = glm::ivec2(8, 16);
        m_DefaultCharacter.bearing = glm::ivec2(0, 0);
        m_DefaultCharacter.advance = 8 << 6;

        m_TextShaper = std::make_unique<TextShaper>();
    }

    FontManager::~FontManager() {
        Shutdown();
    }

    bool FontManager::Init() {
        if (FT_Init_FreeType(&m_FTLibrary)) {
            std::cerr << "[FontManager] Failed to initialize FreeType" << std::endl;
            return false;
        }

        if (m_TextShaper && !m_TextShaper->Init()) {
            std::cerr << "[FontManager] Warning: Failed to initialize TextShaper" << std::endl;
        }

        std::cout << "[FontManager] Initialized successfully with UTF-8 support" << std::endl;
        return true;
    }

    void FontManager::Shutdown() {
        if (m_TextShaper) {
            m_TextShaper->Shutdown();
        }

        // Delete all character textures
        for (auto& [name, fontData] : m_Fonts) {
            for (auto& [codepoint, character] : fontData.characters) {
                if (character.textureID) {
                    glDeleteTextures(1, &character.textureID);
                }
            }
            for (auto& [glyphIndex, character] : fontData.glyphCache) {
                if (character.textureID) {
                    glDeleteTextures(1, &character.textureID);
                }
            }
            if (fontData.face) {
                FT_Done_Face(fontData.face);
            }
        }

        m_Fonts.clear();
        m_ActiveCharacters.clear();
        m_ActiveGlyphCache.clear();
        m_ActiveKerningCache.clear();
        m_ActiveFace = nullptr;

        if (m_FTLibrary) {
            FT_Done_FreeType(m_FTLibrary);
            m_FTLibrary = nullptr;
        }

        std::cout << "[FontManager] Shutdown" << std::endl;
    }

    // ============================================
    // BACKWARD COMPATIBLE: Original LoadFont
    // ============================================
    bool FontManager::LoadFont(const std::string& name, const std::string& filepath,
        uint32_t fontSize) {
        // Call new version with default options
        FontRenderOptions defaultOptions;
        return LoadFontWithOptions(name, filepath, fontSize, defaultOptions);
    }

    // ============================================
    // NEW: Load font with options
    // ============================================
    bool FontManager::LoadFontWithOptions(const std::string& name, const std::string& filepath,
        uint32_t fontSize, const FontRenderOptions& options) {
        FT_Face face;
        if (FT_New_Face(m_FTLibrary, filepath.c_str(), 0, &face)) {
            std::cerr << "[FontManager] Failed to load font: " << filepath << std::endl;
            return false;
        }

        FT_Set_Pixel_Sizes(face, 0, fontSize);

        FontData fontData;
        fontData.fontSize = fontSize;
        fontData.face = face;
        fontData.renderOptions = options;

        // Load basic ASCII
        if (!LoadCharacterRange(face, 0, 127)) {
            std::cerr << "[FontManager] Failed to load ASCII characters" << std::endl;
            FT_Done_Face(face);
            return false;
        }

        // Load Arabic characters
        LoadCharacterRange(face, 0x0600, 0x06FF);
        LoadCharacterRange(face, 0xFB50, 0xFDFF);
        LoadCharacterRange(face, 0xFE70, 0xFEFF);

        fontData.characters = m_ActiveCharacters;

        // Preload kerning if enabled
        if (options.useKerning && FT_HAS_KERNING(face)) {
            std::cout << "[FontManager] Loading kerning pairs for: " << name << std::endl;

            std::vector<std::pair<uint32_t, uint32_t>> commonPairs = {
                {'A', 'V'}, {'A', 'W'}, {'A', 'Y'}, {'T', 'o'}, {'T', 'a'}, {'V', 'a'},
                {'W', 'a'}, {'Y', 'o'}, {'Y', 'a'}, {'r', 'a'}, {'f', 'a'}
            };

            for (const auto& pair : commonPairs) {
                CacheKerning(face, pair.first, pair.second);
            }

            fontData.kerningCache = m_ActiveKerningCache;
        }

        m_Fonts[name] = fontData;

        std::cout << "[FontManager] Loaded font: " << name
            << " with " << fontData.characters.size() << " glyphs"
            << (options.useKerning ? " [Kerning ON]" : "") << std::endl;

        if (m_ActiveFontName.empty()) {
            SetActiveFont(name);
        }

        return true;
    }

    bool FontManager::LoadCharacterRange(FT_Face face, uint32_t start, uint32_t end) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (uint32_t codepoint = start; codepoint <= end; codepoint++) {
            if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
                continue;
            }

            if (face->glyph->bitmap.width == 0 || face->glyph->bitmap.rows == 0) {
                continue;
            }

            uint32_t texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0, GL_RED, GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<uint32_t>(face->glyph->advance.x)
            };

            m_ActiveCharacters[codepoint] = character;
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }

    bool FontManager::LoadUnicodeRange(const std::string& fontName, uint32_t start, uint32_t end) {
        auto it = m_Fonts.find(fontName);
        if (it == m_Fonts.end()) {
            std::cerr << "[FontManager] Font not found: " << fontName << std::endl;
            return false;
        }

        FT_Face face = it->second.face;
        if (!face) {
            std::cerr << "[FontManager] Font face not available: " << fontName << std::endl;
            return false;
        }

        return LoadCharacterRange(face, start, end);
    }

    bool FontManager::SetActiveFont(const std::string& name) {
        auto it = m_Fonts.find(name);
        if (it == m_Fonts.end()) {
            std::cerr << "[FontManager] Font not found: " << name << std::endl;
            return false;
        }

        m_ActiveFontName = name;
        m_ActiveCharacters = it->second.characters;
        m_ActiveGlyphCache = it->second.glyphCache;
        m_ActiveKerningCache = it->second.kerningCache;
        m_ActiveFace = it->second.face;
        m_RenderOptions = it->second.renderOptions;

        std::cout << "[FontManager] Active font set to: " << name
            << " [" << m_ActiveKerningCache.size() << " kerning pairs cached]"
            << std::endl;

        return true;
    }

    const Character& FontManager::GetCharacter(uint32_t codepoint) const {
        auto it = m_ActiveCharacters.find(codepoint);
        if (it != m_ActiveCharacters.end()) {
            return it->second;
        }

        // Try to load dynamically
        if (m_ActiveFace) {
            FontManager* self = const_cast<FontManager*>(this);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            if (FT_Load_Char(m_ActiveFace, codepoint, FT_LOAD_RENDER) == 0) {
                if (m_ActiveFace->glyph->bitmap.width > 0 && m_ActiveFace->glyph->bitmap.rows > 0) {
                    uint32_t texture;
                    glGenTextures(1, &texture);
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glTexImage2D(
                        GL_TEXTURE_2D, 0, GL_RED,
                        m_ActiveFace->glyph->bitmap.width,
                        m_ActiveFace->glyph->bitmap.rows,
                        0, GL_RED, GL_UNSIGNED_BYTE,
                        m_ActiveFace->glyph->bitmap.buffer
                    );

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    Character character = {
                        texture,
                        glm::ivec2(m_ActiveFace->glyph->bitmap.width, m_ActiveFace->glyph->bitmap.rows),
                        glm::ivec2(m_ActiveFace->glyph->bitmap_left, m_ActiveFace->glyph->bitmap_top),
                        static_cast<uint32_t>(m_ActiveFace->glyph->advance.x)
                    };

                    self->m_ActiveCharacters[codepoint] = character;
                    glBindTexture(GL_TEXTURE_2D, 0);
                    return self->m_ActiveCharacters[codepoint];
                }
            }
        }

        return m_DefaultCharacter;
    }

    const Character& FontManager::GetCharacterByGlyphIndex(uint32_t glyphIndex) {
        auto it = m_ActiveGlyphCache.find(glyphIndex);
        if (it != m_ActiveGlyphCache.end()) {
            return it->second;
        }

        if (m_ActiveFace) {
            if (LoadGlyphByIndex(m_ActiveFace, glyphIndex)) {
                auto cachedIt = m_ActiveGlyphCache.find(glyphIndex);
                if (cachedIt != m_ActiveGlyphCache.end()) {
                    return cachedIt->second;
                }
            }
        }

        auto cpIt = m_ActiveCharacters.find(glyphIndex);
        if (cpIt != m_ActiveCharacters.end()) {
            return cpIt->second;
        }

        return m_DefaultCharacter;
    }

    // ============================================
    // NEW: Get kerning
    // ============================================
    float FontManager::GetKerning(uint32_t leftCodepoint, uint32_t rightCodepoint) const {
        if (!m_RenderOptions.useKerning || !m_ActiveFace) {
            return 0.0f;
        }

        if (!FT_HAS_KERNING(m_ActiveFace)) {
            return 0.0f;
        }

        uint64_t key = (static_cast<uint64_t>(leftCodepoint) << 32) | rightCodepoint;

        auto it = m_ActiveKerningCache.find(key);
        if (it != m_ActiveKerningCache.end()) {
            return it->second;
        }

        FT_UInt leftGlyph = FT_Get_Char_Index(m_ActiveFace, leftCodepoint);
        FT_UInt rightGlyph = FT_Get_Char_Index(m_ActiveFace, rightCodepoint);

        if (leftGlyph == 0 || rightGlyph == 0) {
            return 0.0f;
        }

        FT_Vector kerning;
        FT_Error error = FT_Get_Kerning(m_ActiveFace, leftGlyph, rightGlyph,
            FT_KERNING_DEFAULT, &kerning);

        if (error) {
            return 0.0f;
        }

        float kerningValue = kerning.x / 64.0f;

        FontManager* self = const_cast<FontManager*>(this);
        self->m_ActiveKerningCache[key] = kerningValue;

        return kerningValue;
    }

    // ============================================
    // NEW: Cache kerning
    // ============================================
    void FontManager::CacheKerning(FT_Face face, uint32_t left, uint32_t right) {
        if (!FT_HAS_KERNING(face)) {
            return;
        }

        FT_UInt leftGlyph = FT_Get_Char_Index(face, left);
        FT_UInt rightGlyph = FT_Get_Char_Index(face, right);

        if (leftGlyph == 0 || rightGlyph == 0) {
            return;
        }

        FT_Vector kerning;
        FT_Error error = FT_Get_Kerning(face, leftGlyph, rightGlyph,
            FT_KERNING_DEFAULT, &kerning);

        if (error) {
            return;
        }

        float kerningValue = kerning.x / 64.0f;
        uint64_t key = (static_cast<uint64_t>(left) << 32) | right;
        m_ActiveKerningCache[key] = kerningValue;
    }

    // ============================================
    // NEW: Set render options
    // ============================================
    void FontManager::SetRenderOptions(const FontRenderOptions& options) {
        m_RenderOptions = options;

        std::cout << "[FontManager] Render options updated:" << std::endl;
        std::cout << "  - Kerning: " << (options.useKerning ? "ON" : "OFF") << std::endl;
        std::cout << "  - Letter spacing: " << options.letterSpacing << "px" << std::endl;
        std::cout << "  - Line height: " << options.lineHeight << "x" << std::endl;
    }

    // UTF-8 conversion
    uint32_t FontManager::UTF8ToCodepoint(const char*& str) {
        uint32_t codepoint = 0;
        unsigned char c = *str++;

        if (c < 0x80) {
            codepoint = c;
        }
        else if ((c & 0xE0) == 0xC0) {
            codepoint = (c & 0x1F) << 6;
            codepoint |= (*str++ & 0x3F);
        }
        else if ((c & 0xF0) == 0xE0) {
            codepoint = (c & 0x0F) << 12;
            codepoint |= (*str++ & 0x3F) << 6;
            codepoint |= (*str++ & 0x3F);
        }
        else if ((c & 0xF8) == 0xF0) {
            codepoint = (c & 0x07) << 18;
            codepoint |= (*str++ & 0x3F) << 12;
            codepoint |= (*str++ & 0x3F) << 6;
            codepoint |= (*str++ & 0x3F);
        }

        return codepoint;
    }

    size_t FontManager::UTF8CharLength(const char* str) {
        unsigned char c = *str;
        if (c < 0x80) return 1;
        if ((c & 0xE0) == 0xC0) return 2;
        if ((c & 0xF0) == 0xE0) return 3;
        if ((c & 0xF8) == 0xF0) return 4;
        return 1;
    }

    glm::vec2 FontManager::CalculateTextSize(const std::string& utf8Text, float scale) const {
        float width = 0.0f;
        float height = 0.0f;

        const char* str = utf8Text.c_str();
        while (*str) {
            uint32_t codepoint = UTF8ToCodepoint(str);
            const Character& ch = GetCharacter(codepoint);

            width += (ch.advance >> 6) * scale;
            height = glm::max(height, (float)ch.size.y * scale);
        }

        return glm::vec2(width, height);
    }

    bool FontManager::LoadCharacters(FT_Face face, uint32_t fontSize) {
        return LoadCharacterRange(face, 0, 127);
    }

    // Helper functions for BiDi
    static bool IsRTLCodepoint(uint32_t codepoint) {
        return (codepoint >= 0x0600 && codepoint <= 0x06FF) ||
            (codepoint >= 0xFB50 && codepoint <= 0xFDFF) ||
            (codepoint >= 0xFE70 && codepoint <= 0xFEFF) ||
            (codepoint >= 0x0590 && codepoint <= 0x05FF);
    }

    static bool IsWhitespace(uint32_t codepoint) {
        return codepoint == 0x0020 || codepoint == 0x00A0 || codepoint == 0x0009;
    }

    std::vector<ShapedGlyph> FontManager::ShapeText(const std::string& utf8Text) {
        std::vector<ShapedGlyph> allGlyphs;

        // Try HarfBuzz first
        if (m_TextShaper && m_ActiveFace) {
            m_TextShaper->SetFont(m_ActiveFace);
            auto hbGlyphs = m_TextShaper->ShapeText(utf8Text);

            if (!hbGlyphs.empty()) {
                return hbGlyphs;
            }
        }

        // Fallback with BiDi
        std::vector<uint32_t> codepoints;
        const char* str = utf8Text.c_str();
        while (*str) {
            codepoints.push_back(UTF8ToCodepoint(str));
        }

        if (codepoints.empty()) {
            return allGlyphs;
        }

        struct TextRun {
            std::vector<uint32_t> codepoints;
            bool isRTL;
        };

        std::vector<TextRun> runs;
        TextRun currentRun;
        currentRun.isRTL = IsRTLCodepoint(codepoints[0]);
        currentRun.codepoints.push_back(codepoints[0]);

        for (size_t i = 1; i < codepoints.size(); i++) {
            uint32_t cp = codepoints[i];
            bool cpIsRTL = IsRTLCodepoint(cp);
            bool isSpace = IsWhitespace(cp);

            if (isSpace) {
                currentRun.codepoints.push_back(cp);
            }
            else if (cpIsRTL != currentRun.isRTL) {
                runs.push_back(currentRun);
                currentRun = TextRun();
                currentRun.isRTL = cpIsRTL;
                currentRun.codepoints.push_back(cp);
            }
            else {
                currentRun.codepoints.push_back(cp);
            }
        }

        if (!currentRun.codepoints.empty()) {
            runs.push_back(currentRun);
        }

        bool hasMixedText = runs.size() > 1;
        bool startsWithRTL = !runs.empty() && runs[0].isRTL;

        if (hasMixedText && startsWithRTL) {
            std::reverse(runs.begin(), runs.end());
        }

        float xPos = 0.0f;

        for (auto& run : runs) {
            if (run.isRTL) {
                std::reverse(run.codepoints.begin(), run.codepoints.end());
            }

            for (uint32_t cp : run.codepoints) {
                const Character& ch = GetCharacter(cp);

                ShapedGlyph glyph;
                glyph.glyphIndex = cp;
                glyph.codepoint = cp;
                glyph.offset = glm::vec2(xPos, 0.0f);
                glyph.advance = glm::vec2((ch.advance >> 6), 0.0f);

                allGlyphs.push_back(glyph);
                xPos += glyph.advance.x;
            }
        }

        return allGlyphs;
    }

    bool FontManager::LoadGlyphByIndex(FT_Face face, uint32_t glyphIndex) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER) != 0) {
            return false;
        }

        if (face->glyph->bitmap.width == 0 || face->glyph->bitmap.rows == 0) {
            return false;
        }

        uint32_t texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
            face->glyph->bitmap.width, face->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<uint32_t>(face->glyph->advance.x)
        };

        m_ActiveGlyphCache[glyphIndex] = character;
        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }

    uint32_t FontManager::GenerateCharacterTexture(FT_Face face, uint32_t codepoint) {
        // This will be used for enhanced AA in future
        return 0;
    }

} // namespace Unicorn::UI