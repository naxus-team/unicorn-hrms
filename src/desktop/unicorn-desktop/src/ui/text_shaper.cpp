// ========================================
// text_shaper.cpp - Universal Language + Emoji Support
// Complete solution for ALL languages and emoji
// ========================================

#include "text_shaper.h"
#include <iostream>
#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

namespace Unicorn::UI {

    // ========================================
    // Universal Character Classification
    // ========================================

    enum class ScriptType {
        Unknown,
        Latin,      // English, French, German, Spanish, etc.
        Arabic,     // Arabic
        Hebrew,     // Hebrew
        Cyrillic,   // Russian, Ukrainian, Serbian, etc.
        Greek,      // Greek
        Thai,       // Thai
        Devanagari, // Hindi, Sanskrit, Marathi
        Bengali,    // Bengali
        Tamil,      // Tamil
        Telugu,     // Telugu
        Kannada,    // Kannada
        Malayalam,  // Malayalam
        Gujarati,   // Gujarati
        CJK,        // Chinese, Japanese, Korean
        Emoji,      // Emoji (⭐ NEW)
        Digit,      // Numbers (0-9)
        Neutral     // Spaces, punctuation
    };

    static ScriptType GetScriptType(uint32_t cp) {
        // ⭐ EMOJI DETECTION (HIGHEST PRIORITY)
        // Basic Emoji & Pictographs
        if ((cp >= 0x1F300 && cp <= 0x1F9FF) ||  // Misc Symbols and Pictographs + Supplemental
            (cp >= 0x2600 && cp <= 0x26FF) ||    // Misc Symbols
            (cp >= 0x2700 && cp <= 0x27BF) ||    // Dingbats
            (cp >= 0xFE00 && cp <= 0xFE0F) ||    // Variation Selectors
            (cp >= 0x1F000 && cp <= 0x1F02F) ||  // Mahjong Tiles
            (cp >= 0x1F0A0 && cp <= 0x1F0FF) ||  // Playing Cards
            (cp >= 0x1FA70 && cp <= 0x1FAFF) ||  // Symbols and Pictographs Extended-A
            // Emoji modifiers
            (cp >= 0x1F3FB && cp <= 0x1F3FF) ||  // Skin tone modifiers
            // Zero-width joiner for emoji sequences
            (cp == 0x200D) ||                     // ZWJ
            // Regional indicator symbols (flags)
            (cp >= 0x1F1E6 && cp <= 0x1F1FF))    // Regional Indicators
            return ScriptType::Emoji;

        // Digits (ALWAYS LTR)
        if (cp >= 0x0030 && cp <= 0x0039) return ScriptType::Digit;

        // Neutral characters
        if (cp == 0x0020 || cp == 0x003A || cp == 0x002C ||
            cp == 0x002E || cp == 0x002D || cp == 0x0028 ||
            cp == 0x0029 || cp == 0x003D || cp == 0x0022)
            return ScriptType::Neutral;

        // Latin (English, French, Spanish, German, etc.)
        if ((cp >= 0x0041 && cp <= 0x005A) ||  // A-Z
            (cp >= 0x0061 && cp <= 0x007A) ||  // a-z
            (cp >= 0x00C0 && cp <= 0x00FF) ||  // Latin Extended (À, É, Ñ, etc.)
            (cp >= 0x0100 && cp <= 0x017F) ||  // Latin Extended-A
            (cp >= 0x0180 && cp <= 0x024F))    // Latin Extended-B
            return ScriptType::Latin;

        // Arabic
        if ((cp >= 0x0600 && cp <= 0x06FF) ||  // Arabic
            (cp >= 0xFB50 && cp <= 0xFDFF) ||  // Arabic Presentation Forms-A
            (cp >= 0xFE70 && cp <= 0xFEFF))    // Arabic Presentation Forms-B
            return ScriptType::Arabic;

        // Hebrew
        if (cp >= 0x0590 && cp <= 0x05FF) return ScriptType::Hebrew;

        // Cyrillic (Russian, Ukrainian, etc.)
        if ((cp >= 0x0400 && cp <= 0x04FF) ||  // Cyrillic
            (cp >= 0x0500 && cp <= 0x052F))    // Cyrillic Supplement
            return ScriptType::Cyrillic;

        // Greek
        if ((cp >= 0x0370 && cp <= 0x03FF) ||  // Greek
            (cp >= 0x1F00 && cp <= 0x1FFF))    // Greek Extended
            return ScriptType::Greek;

        // Thai
        if (cp >= 0x0E00 && cp <= 0x0E7F) return ScriptType::Thai;

        // Devanagari (Hindi, Sanskrit, Marathi)
        if (cp >= 0x0900 && cp <= 0x097F) return ScriptType::Devanagari;

        // Bengali
        if (cp >= 0x0980 && cp <= 0x09FF) return ScriptType::Bengali;

        // Tamil
        if (cp >= 0x0B80 && cp <= 0x0BFF) return ScriptType::Tamil;

        // Telugu
        if (cp >= 0x0C00 && cp <= 0x0C7F) return ScriptType::Telugu;

        // Kannada
        if (cp >= 0x0C80 && cp <= 0x0CFF) return ScriptType::Kannada;

        // Malayalam
        if (cp >= 0x0D00 && cp <= 0x0D7F) return ScriptType::Malayalam;

        // Gujarati
        if (cp >= 0x0A80 && cp <= 0x0AFF) return ScriptType::Gujarati;

        // ⭐ CJK (Chinese, Japanese, Korean) - EXPANDED
        if ((cp >= 0x4E00 && cp <= 0x9FFF) ||   // CJK Unified Ideographs
            (cp >= 0x3400 && cp <= 0x4DBF) ||   // CJK Extension A
            (cp >= 0x20000 && cp <= 0x2A6DF) || // CJK Extension B
            (cp >= 0x2A700 && cp <= 0x2B73F) || // CJK Extension C
            (cp >= 0x2B740 && cp <= 0x2B81F) || // CJK Extension D
            (cp >= 0x2B820 && cp <= 0x2CEAF) || // CJK Extension E
            (cp >= 0xF900 && cp <= 0xFAFF) ||   // CJK Compatibility Ideographs
            (cp >= 0x3040 && cp <= 0x309F) ||   // Hiragana
            (cp >= 0x30A0 && cp <= 0x30FF) ||   // Katakana
            (cp >= 0x31F0 && cp <= 0x31FF) ||   // Katakana Phonetic Extensions
            (cp >= 0x3190 && cp <= 0x319F) ||   // Kanbun
            (cp >= 0xAC00 && cp <= 0xD7AF) ||   // Hangul Syllables
            (cp >= 0x1100 && cp <= 0x11FF))     // Hangul Jamo
            return ScriptType::CJK;

        return ScriptType::Unknown;
    }

    static bool IsRTL(ScriptType script) {
        return script == ScriptType::Arabic || script == ScriptType::Hebrew;
    }

    static hb_script_t GetHarfBuzzScript(ScriptType script) {
        switch (script) {
        case ScriptType::Latin: return HB_SCRIPT_LATIN;
        case ScriptType::Arabic: return HB_SCRIPT_ARABIC;
        case ScriptType::Hebrew: return HB_SCRIPT_HEBREW;
        case ScriptType::Cyrillic: return HB_SCRIPT_CYRILLIC;
        case ScriptType::Greek: return HB_SCRIPT_GREEK;
        case ScriptType::Thai: return HB_SCRIPT_THAI;
        case ScriptType::Devanagari: return HB_SCRIPT_DEVANAGARI;
        case ScriptType::Bengali: return HB_SCRIPT_BENGALI;
        case ScriptType::Tamil: return HB_SCRIPT_TAMIL;
        case ScriptType::Telugu: return HB_SCRIPT_TELUGU;
        case ScriptType::Kannada: return HB_SCRIPT_KANNADA;
        case ScriptType::Malayalam: return HB_SCRIPT_MALAYALAM;
        case ScriptType::Gujarati: return HB_SCRIPT_GUJARATI;
        case ScriptType::CJK: return HB_SCRIPT_HAN;
        case ScriptType::Emoji: return HB_SCRIPT_COMMON; // ⭐ Emoji use COMMON script
        case ScriptType::Digit: return HB_SCRIPT_COMMON;
        default: return HB_SCRIPT_COMMON;
        }
    }

    static const char* GetHarfBuzzLanguage(ScriptType script) {
        switch (script) {
        case ScriptType::Latin: return "en";
        case ScriptType::Arabic: return "ar";
        case ScriptType::Hebrew: return "he";
        case ScriptType::Cyrillic: return "ru";
        case ScriptType::Greek: return "el";
        case ScriptType::Thai: return "th";
        case ScriptType::Devanagari: return "hi";
        case ScriptType::Bengali: return "bn";
        case ScriptType::Tamil: return "ta";
        case ScriptType::Telugu: return "te";
        case ScriptType::Kannada: return "kn";
        case ScriptType::Malayalam: return "ml";
        case ScriptType::Gujarati: return "gu";
        case ScriptType::CJK: return "zh";
        case ScriptType::Emoji: return "und"; // ⭐ Undetermined for emoji
        default: return "en";
        }
    }

    // ========================================
    // UTF-8 Helper
    // ========================================
    static uint32_t UTF8ToCodepoint(const char*& str) {
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

    // ========================================
    // TextShaper Implementation
    // ========================================

    TextShaper::TextShaper() {}

    TextShaper::~TextShaper() {
        Shutdown();
    }

    bool TextShaper::Init() {
        m_HBBuffer = hb_buffer_create();
        if (!m_HBBuffer) {
            std::cerr << "[TextShaper] Failed to create HarfBuzz buffer" << std::endl;
            return false;
        }
        std::cout << "[TextShaper] Initialized with universal language + emoji support" << std::endl;
        return true;
    }

    void TextShaper::Shutdown() {
        if (m_HBFont) {
            hb_font_destroy(m_HBFont);
            m_HBFont = nullptr;
        }

        if (m_HBBuffer) {
            hb_buffer_destroy(m_HBBuffer);
            m_HBBuffer = nullptr;
        }

        m_Face = nullptr;
    }

    bool TextShaper::SetFont(FT_Face face) {
        if (!face) {
            std::cerr << "[TextShaper] Invalid FreeType face" << std::endl;
            return false;
        }

        m_Face = face;

        if (m_HBFont) {
            hb_font_destroy(m_HBFont);
        }

        m_HBFont = hb_ft_font_create(face, nullptr);
        if (!m_HBFont) {
            std::cerr << "[TextShaper] Failed to create HarfBuzz font" << std::endl;
            return false;
        }

        return true;
    }

    std::vector<ShapedGlyph> TextShaper::ShapeText(const std::string& utf8Text) {
        std::vector<ShapedGlyph> allGlyphs;

        if (utf8Text.empty() || !m_HBFont || !m_HBBuffer) {
            return allGlyphs;
        }

        // ========================================
        // Step 1: Convert to codepoints
        // ========================================
        struct TextSegment {
            size_t start;
            size_t length;
            ScriptType script;
            std::string text;
        };

        std::vector<TextSegment> segments;
        std::vector<uint32_t> codepoints;
        std::vector<size_t> positions;

        const char* str = utf8Text.c_str();
        const char* strStart = str;
        while (*str) {
            positions.push_back(str - strStart);
            codepoints.push_back(UTF8ToCodepoint(str));
        }
        positions.push_back(str - strStart);

        if (codepoints.empty()) {
            return allGlyphs;
        }

        // ========================================
        // Step 2: Segment by script
        // ========================================
        size_t currentStart = 0;
        ScriptType currentScript = GetScriptType(codepoints[0]);

        for (size_t i = 1; i <= codepoints.size(); i++) {
            bool shouldBreak = false;

            if (i == codepoints.size()) {
                shouldBreak = true;
            }
            else {
                ScriptType cpScript = GetScriptType(codepoints[i]);

                // ⭐ CRITICAL: Don't merge different scripts
                // Neutral characters inherit the current script
                if (cpScript == ScriptType::Neutral) {
                    // Keep current script
                }
                else if (cpScript != currentScript) {
                    shouldBreak = true;
                }
            }

            if (shouldBreak) {
                TextSegment seg;
                seg.start = positions[currentStart];
                seg.length = positions[i] - positions[currentStart];
                seg.text = utf8Text.substr(seg.start, seg.length);
                seg.script = currentScript;

                segments.push_back(seg);

                if (i < codepoints.size()) {
                    currentStart = i;
                    ScriptType newScript = GetScriptType(codepoints[i]);
                    currentScript = (newScript == ScriptType::Neutral) ? currentScript : newScript;
                }
            }
        }

        // ========================================
        // Step 3: Determine overall direction
        // ========================================
        int rtlCount = 0;
        int ltrCount = 0;

        for (const auto& seg : segments) {
            if (IsRTL(seg.script)) rtlCount++;
            else if (seg.script != ScriptType::Neutral && seg.script != ScriptType::Emoji) ltrCount++;
        }

        bool overallRTL = rtlCount > ltrCount;

        // ========================================
        // Step 4: Reverse segment order if RTL (BiDi)
        // ========================================
        if (overallRTL && segments.size() > 1) {
            std::reverse(segments.begin(), segments.end());
        }

        // ========================================
        // Step 5: Shape each segment
        // ========================================
        float currentX = 0.0f;

        for (const auto& segment : segments) {
            if (segment.text.empty()) continue;

            hb_buffer_reset(m_HBBuffer);

            // ⭐ Set direction and script CORRECTLY for each segment
            bool segmentIsRTL = IsRTL(segment.script);

            hb_buffer_set_direction(m_HBBuffer,
                segmentIsRTL ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);

            hb_buffer_set_script(m_HBBuffer, GetHarfBuzzScript(segment.script));
            hb_buffer_set_language(m_HBBuffer,
                hb_language_from_string(GetHarfBuzzLanguage(segment.script), -1));

            // Add text to buffer
            hb_buffer_add_utf8(m_HBBuffer,
                segment.text.c_str(),
                segment.text.length(),
                0,
                segment.text.length());

            // ⭐ CRITICAL: Enable emoji and ligature features
            hb_feature_t features[10];
            int featureCount = 0;

            // Enable ligatures for all scripts
            features[featureCount++] = { HB_TAG('l','i','g','a'), 1, 0, (unsigned int)-1 };
            features[featureCount++] = { HB_TAG('c','l','i','g'), 1, 0, (unsigned int)-1 };

            // Enable emoji features
            if (segment.script == ScriptType::Emoji) {
                features[featureCount++] = { HB_TAG('c','o','l','r'), 1, 0, (unsigned int)-1 };
                features[featureCount++] = { HB_TAG('C','O','L','R'), 1, 0, (unsigned int)-1 };
            }

            // Shape with features
            hb_shape(m_HBFont, m_HBBuffer, features, featureCount);

            // Get results
            unsigned int glyphCount;
            hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(m_HBBuffer, &glyphCount);
            hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(m_HBBuffer, &glyphCount);

            // Convert to our format
            for (unsigned int i = 0; i < glyphCount; i++) {
                ShapedGlyph glyph;
                glyph.glyphIndex = glyphInfo[i].codepoint;
                glyph.codepoint = glyphInfo[i].codepoint;
                glyph.offset.x = (glyphPos[i].x_offset / 64.0f) + currentX;
                glyph.offset.y = glyphPos[i].y_offset / 64.0f;
                glyph.advance.x = glyphPos[i].x_advance / 64.0f;
                glyph.advance.y = glyphPos[i].y_advance / 64.0f;

                allGlyphs.push_back(glyph);
                currentX += glyph.advance.x;
            }
        }

        return allGlyphs;
    }

    glm::vec2 TextShaper::CalculateTextSize(const std::string& utf8Text) {
        auto shapedGlyphs = ShapeText(utf8Text);

        float width = 0.0f;
        float height = 0.0f;

        for (const auto& glyph : shapedGlyphs) {
            width += glyph.advance.x;

            if (m_Face && FT_Load_Glyph(m_Face, glyph.glyphIndex, FT_LOAD_DEFAULT) == 0) {
                height = std::max(height, (float)m_Face->glyph->metrics.height / 64.0f);
            }
        }

        return glm::vec2(width, height > 0 ? height : 16.0f);
    }

} // namespace Unicorn::UI