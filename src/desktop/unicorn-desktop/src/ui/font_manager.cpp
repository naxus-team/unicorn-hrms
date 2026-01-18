#include "font_manager.h"
#include <glad/glad.h>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Unicorn::UI {

    // ========================================
    // Font Atlas Implementation
    // ========================================

    bool FontAtlas::AddGlyph(uint32_t glyphWidth, uint32_t glyphHeight,
        const unsigned char* pixelData,
        glm::vec2& outAtlasPos, glm::vec2& outAtlasSize) {
        if (currentX + glyphWidth > width) {
            currentX = 0;
            currentY += rowHeight;
            rowHeight = 0;
        }

        if (currentY + glyphHeight > height) {
            std::cerr << "[FontAtlas] Atlas is full!" << std::endl;
            return false;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, currentX, currentY, glyphWidth, glyphHeight,
            GL_RED, GL_UNSIGNED_BYTE, pixelData);

        outAtlasPos = glm::vec2((float)currentX / (float)width, (float)currentY / (float)height);
        outAtlasSize = glm::vec2((float)glyphWidth / (float)width, (float)glyphHeight / (float)height);

        currentX += glyphWidth;
        rowHeight = glm::max(rowHeight, glyphHeight);

        return true;
    }

    void FontAtlas::Clear() {
        currentX = 0;
        currentY = 0;
        rowHeight = 0;

        if (textureID != 0) {
            glBindTexture(GL_TEXTURE_2D, textureID);
            std::vector<unsigned char> clearData(width * height, 0);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, clearData.data());
        }
    }

    // ========================================
    // FontManager Implementation
    // ========================================

    FontManager::FontManager() {
        m_DefaultCharacter.textureID = 0;
        m_DefaultCharacter.size = glm::ivec2(8, 16);
        m_DefaultCharacter.bearing = glm::ivec2(0, 0);
        m_DefaultCharacter.advance = 8 << 6;
        m_DefaultCharacter.atlasPos = glm::vec2(0, 0);
        m_DefaultCharacter.atlasSize = glm::vec2(0, 0);

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

        // Create atlas texture
        glGenTextures(1, &m_Atlas.textureID);
        glBindTexture(GL_TEXTURE_2D, m_Atlas.textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_Atlas.width, m_Atlas.height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        std::cout << "[FontManager] Font Atlas created: " << m_Atlas.width << "x" << m_Atlas.height << std::endl;
        std::cout << "[FontManager] Initialized successfully" << std::endl;
        return true;
    }

    void FontManager::Shutdown() {
        if (m_TextShaper) {
            m_TextShaper->Shutdown();
        }

        if (m_Atlas.textureID) {
            glDeleteTextures(1, &m_Atlas.textureID);
            m_Atlas.textureID = 0;
        }

        for (auto& [name, fontData] : m_Fonts) {
            for (auto& [codepoint, character] : fontData.characters) {
                if (character.textureID != m_Atlas.textureID && character.textureID) {
                    glDeleteTextures(1, &character.textureID);
                }
            }
            for (auto& [glyphIndex, character] : fontData.glyphCache) {
                if (character.textureID != m_Atlas.textureID && character.textureID) {
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

    bool FontManager::LoadFont(const std::string& name, const std::string& filepath, uint32_t fontSize) {
        FontRenderOptions defaultOptions;
        return LoadFontWithOptions(name, filepath, fontSize, defaultOptions);
    }

    bool FontManager::LoadFontWithOptions(const std::string& name, const std::string& filepath,
        uint32_t fontSize, const FontRenderOptions& options) {

        std::cout << "[FontManager] Loading: " << name << " from " << filepath << std::endl;

        std::ifstream fileCheck(filepath);
        if (!fileCheck.good()) {
            std::cerr << "[FontManager] File not found: " << filepath << std::endl;
            return false;
        }
        fileCheck.close();

        FT_Face face;
        FT_Error error = FT_New_Face(m_FTLibrary, filepath.c_str(), 0, &face);
        if (error) {
            std::cerr << "[FontManager] Failed to load font, error: " << error << std::endl;
            return false;
        }

        std::cout << "[FontManager] Font: " << (face->family_name ? face->family_name : "Unknown")
            << " (" << face->num_glyphs << " glyphs)" << std::endl;

        uint32_t renderSize = fontSize * 2;
        FT_Set_Pixel_Sizes(face, 0, renderSize);

        int32_t loadFlags = FT_LOAD_DEFAULT | FT_LOAD_FORCE_AUTOHINT;
        if (options.useHinting) {
            loadFlags |= FT_LOAD_TARGET_LIGHT;
        }
        else {
            loadFlags |= FT_LOAD_NO_HINTING;
        }

        std::unordered_map<uint32_t, Character> tempCharacters;
        std::unordered_map<uint32_t, Character> tempGlyphCache;
        std::unordered_map<uint64_t, float> tempKerningCache;

        auto loadRange = [&](uint32_t start, uint32_t end, const char* rangeName) -> bool {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            uint32_t loadedCount = 0;

            for (uint32_t codepoint = start; codepoint <= end; codepoint++) {
                uint32_t glyphIndex = FT_Get_Char_Index(face, codepoint);
                if (glyphIndex == 0) continue;
                if (FT_Load_Glyph(face, glyphIndex, loadFlags)) continue;

                FT_Render_Mode renderMode = options.useAntialiasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
                if (FT_Render_Glyph(face->glyph, renderMode)) continue;

                if (face->glyph->bitmap.width == 0 || face->glyph->bitmap.rows == 0) {
                    Character character;
                    character.textureID = m_Atlas.textureID;
                    character.atlasPos = glm::vec2(0, 0);
                    character.atlasSize = glm::vec2(0, 0);
                    character.size = glm::ivec2(0, 0);
                    character.bearing = glm::ivec2(0, 0);
                    character.advance = static_cast<uint32_t>(face->glyph->advance.x);
                    tempCharacters[codepoint] = character;
                    loadedCount++;
                    continue;
                }

                glm::vec2 atlasPos, atlasSize;
                if (m_Atlas.AddGlyph(
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    face->glyph->bitmap.buffer,
                    atlasPos, atlasSize
                )) {
                    float scale = (float)fontSize / (float)renderSize;
                    Character character;
                    character.textureID = m_Atlas.textureID;
                    character.atlasPos = atlasPos;
                    character.atlasSize = atlasSize;
                    character.size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
                    character.bearing = glm::ivec2(
                        static_cast<int>(face->glyph->bitmap_left * scale),
                        static_cast<int>(face->glyph->bitmap_top * scale)
                    );
                    character.advance = static_cast<uint32_t>(face->glyph->advance.x * scale);
                    tempCharacters[codepoint] = character;
                    loadedCount++;
                }
            }

            std::cout << "[FontManager]   " << rangeName << ": " << loadedCount << " chars" << std::endl;
            return loadedCount > 0;
            };

        if (!loadRange(0, 127, "ASCII")) {
            std::cerr << "[FontManager] CRITICAL: Failed to load ASCII" << std::endl;
            FT_Done_Face(face);
            return false;
        }

        loadRange(128, 255, "Extended ASCII");
        loadRange(0x0100, 0x017F, "Latin Extended-A");
        loadRange(0x0180, 0x024F, "Latin Extended-B");
        loadRange(0x0600, 0x06FF, "Arabic");
        loadRange(0xFB50, 0xFDFF, "Arabic Forms-A");
        loadRange(0xFE70, 0xFEFF, "Arabic Forms-B");
        loadRange(0x0590, 0x05FF, "Hebrew");
        loadRange(0x0400, 0x04FF, "Cyrillic");
        loadRange(0x0370, 0x03FF, "Greek");
        loadRange(0x2000, 0x206F, "Punctuation");
        loadRange(0x20A0, 0x20CF, "Currency");

        FontData fontData;
        fontData.fontSize = fontSize;
        fontData.face = face;
        fontData.renderOptions = options;
        fontData.characters = std::move(tempCharacters);
        fontData.glyphCache = std::move(tempGlyphCache);
        fontData.kerningCache = std::move(tempKerningCache);
        m_Fonts[name] = std::move(fontData);

        std::cout << "[FontManager] Loaded " << m_Fonts[name].characters.size() << " characters into atlas" << std::endl;

        if (m_ActiveFontName.empty()) {
            SetActiveFont(name);
        }
        return true;
    }

    bool FontManager::LoadCharacterRange(FT_Face face, uint32_t start, uint32_t end) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        uint32_t loadedCount = 0;

        for (uint32_t codepoint = start; codepoint <= end; codepoint++) {
            uint32_t glyphIndex = FT_Get_Char_Index(face, codepoint);
            if (glyphIndex == 0) continue;

            if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
                continue;
            }

            if (face->glyph->bitmap.width == 0 || face->glyph->bitmap.rows == 0) {
                Character character;
                character.textureID = m_Atlas.textureID;
                character.atlasPos = glm::vec2(0, 0);
                character.atlasSize = glm::vec2(0, 0);
                character.size = glm::ivec2(0, 0);  // FIX
                character.bearing = glm::ivec2(0, 0);  // FIX
                character.advance = static_cast<uint32_t>(face->glyph->advance.x);
                m_ActiveCharacters[codepoint] = character;
                loadedCount++;
                continue;
            }

            glm::vec2 atlasPos, atlasSize;
            if (m_Atlas.AddGlyph(
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                face->glyph->bitmap.buffer,
                atlasPos, atlasSize
            )) {
                Character character;
                character.textureID = m_Atlas.textureID;
                character.atlasPos = atlasPos;
                character.atlasSize = atlasSize;
                character.size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);  // FIX
                character.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);  // FIX
                character.advance = static_cast<uint32_t>(face->glyph->advance.x);
                m_ActiveCharacters[codepoint] = character;
                loadedCount++;
            }
        }

        return true;
    }

    bool FontManager::LoadUnicodeRange(const std::string& fontName, uint32_t start, uint32_t end) {
        auto it = m_Fonts.find(fontName);
        if (it == m_Fonts.end()) {
            return false;
        }

        return LoadCharacterRange(it->second.face, start, end);
    }

    bool FontManager::SetActiveFont(const std::string& name) {
        auto it = m_Fonts.find(name);
        if (it == m_Fonts.end()) {
            return false;
        }

        m_ActiveFontName = name;
        m_ActiveCharacters = it->second.characters;
        m_ActiveGlyphCache = it->second.glyphCache;
        m_ActiveKerningCache = it->second.kerningCache;
        m_ActiveFace = it->second.face;
        m_RenderOptions = it->second.renderOptions;

        std::cout << "[FontManager] Active font: " << name << " | "
            << m_ActiveCharacters.size() << " characters" << std::endl;

        return true;
    }

    const Character& FontManager::GetCharacter(uint32_t codepoint) const {
        auto it = m_ActiveCharacters.find(codepoint);
        if (it != m_ActiveCharacters.end()) {
            return it->second;
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

    float FontManager::GetKerning(uint32_t leftCodepoint, uint32_t rightCodepoint) const {
        if (!m_RenderOptions.useKerning || !m_ActiveFace || !FT_HAS_KERNING(m_ActiveFace)) {
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
        FT_Error error = FT_Get_Kerning(m_ActiveFace, leftGlyph, rightGlyph, FT_KERNING_DEFAULT, &kerning);
        if (error) {
            return 0.0f;
        }

        float kerningValue = kerning.x / 64.0f;
        FontManager* self = const_cast<FontManager*>(this);
        self->m_ActiveKerningCache[key] = kerningValue;

        return kerningValue;
    }

    void FontManager::CacheKerning(FT_Face face, uint32_t left, uint32_t right) {
        if (!FT_HAS_KERNING(face)) return;

        FT_UInt leftGlyph = FT_Get_Char_Index(face, left);
        FT_UInt rightGlyph = FT_Get_Char_Index(face, right);
        if (leftGlyph == 0 || rightGlyph == 0) return;

        FT_Vector kerning;
        if (FT_Get_Kerning(face, leftGlyph, rightGlyph, FT_KERNING_DEFAULT, &kerning) == 0) {
            float kerningValue = kerning.x / 64.0f;
            uint64_t key = (static_cast<uint64_t>(left) << 32) | right;
            m_ActiveKerningCache[key] = kerningValue;
        }
    }

    void FontManager::SetRenderOptions(const FontRenderOptions& options) {
        m_RenderOptions = options;
    }

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

        if (m_TextShaper && m_ActiveFace) {
            m_TextShaper->SetFont(m_ActiveFace);
            auto hbGlyphs = m_TextShaper->ShapeText(utf8Text);
            if (!hbGlyphs.empty()) {
                return hbGlyphs;
            }
        }

        std::vector<uint32_t> codepoints;
        const char* str = utf8Text.c_str();
        while (*str) {
            codepoints.push_back(UTF8ToCodepoint(str));
        }

        if (codepoints.empty()) return allGlyphs;

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

        glm::vec2 atlasPos, atlasSize;
        if (m_Atlas.AddGlyph(
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            face->glyph->bitmap.buffer,
            atlasPos, atlasSize
        )) {
            Character character;
            character.textureID = m_Atlas.textureID;
            character.atlasPos = atlasPos;
            character.atlasSize = atlasSize;
            character.size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);  // FIX
            character.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);  // FIX
            character.advance = static_cast<uint32_t>(face->glyph->advance.x);
            m_ActiveGlyphCache[glyphIndex] = character;
            return true;
        }

        return false;
    }

    uint32_t FontManager::GenerateCharacterTexture(FT_Face face, uint32_t codepoint) {
        return 0;
    }

} // namespace Unicorn::UI