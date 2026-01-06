#include "text_shaper.h"
#include <iostream>
#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

namespace Unicorn::UI {

    // Helper functions
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

    static bool IsRTLCodepoint(uint32_t codepoint) {
        return (codepoint >= 0x0600 && codepoint <= 0x06FF) ||  // Arabic
            (codepoint >= 0xFB50 && codepoint <= 0xFDFF) ||  // Arabic Presentation Forms-A
            (codepoint >= 0xFE70 && codepoint <= 0xFEFF) ||  // Arabic Presentation Forms-B
            (codepoint >= 0x0590 && codepoint <= 0x05FF);    // Hebrew
    }

    static bool IsLTRCodepoint(uint32_t codepoint) {
        return (codepoint >= 0x0041 && codepoint <= 0x005A) ||  // A-Z
            (codepoint >= 0x0061 && codepoint <= 0x007A);    // a-z
    }

    // ========================================
    // NEW: Check if character is a digit (0-9)
    // ========================================
    static bool IsDigit(uint32_t codepoint) {
        return (codepoint >= 0x0030 && codepoint <= 0x0039);  // 0-9
    }

    static bool IsNeutralChar(uint32_t codepoint) {
        // Digits are NO LONGER neutral - they're LTR
        return codepoint == 0x0020 ||  // Space
            codepoint == 0x003A ||  // :
            codepoint == 0x002C ||  // ,
            codepoint == 0x002E ||  // .
            codepoint == 0x002D;    // -
    }

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
        std::cout << "[TextShaper] Initialized with HarfBuzz support" << std::endl;
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
        std::cout << "[TextShaper] Shutdown" << std::endl;
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

        // Step 1: Analyze text to detect runs
        struct TextSegment {
            size_t start;
            size_t length;
            bool isRTL;
            bool isDigits;  // NEW: Track if segment is all digits
            std::string text;
        };

        std::vector<TextSegment> segments;
        std::vector<uint32_t> codepoints;
        std::vector<size_t> positions;

        // Convert text to codepoints
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

        // Step 2: Segment text into directional runs
        size_t currentStart = 0;
        bool currentIsRTL = IsRTLCodepoint(codepoints[0]);
        bool currentIsDigits = IsDigit(codepoints[0]);

        // Count RTL vs LTR
        int rtlCount = 0;
        int ltrCount = 0;
        for (auto cp : codepoints) {
            if (IsRTLCodepoint(cp)) rtlCount++;
            else if (IsLTRCodepoint(cp) || IsDigit(cp)) ltrCount++;
        }

        bool overallRTL = rtlCount > ltrCount;

        for (size_t i = 1; i <= codepoints.size(); i++) {
            bool shouldBreak = false;

            if (i == codepoints.size()) {
                shouldBreak = true;
            }
            else {
                uint32_t cp = codepoints[i];
                bool cpIsRTL = IsRTLCodepoint(cp);
                bool cpIsDigit = IsDigit(cp);
                bool cpIsNeutral = IsNeutralChar(cp);

                if (cpIsDigit != currentIsDigits && !cpIsNeutral) {
                    shouldBreak = true;
                }
                else if (!cpIsNeutral && !cpIsDigit && cpIsRTL != currentIsRTL) {
                    shouldBreak = true;
                }
            }

            if (shouldBreak) {
                TextSegment seg;
                seg.start = positions[currentStart];
                seg.length = positions[i] - positions[currentStart];
                seg.text = utf8Text.substr(seg.start, seg.length);

                // Check if all digits
                seg.isDigits = true;
                for (size_t j = currentStart; j < i; j++) {
                    if (!IsDigit(codepoints[j]) && !IsNeutralChar(codepoints[j])) {
                        seg.isDigits = false;
                        break;
                    }
                }

                if (seg.isDigits) {
                    seg.isRTL = false;
                }
                else {
                    seg.isRTL = currentIsRTL;
                }

                segments.push_back(seg);

                if (i < codepoints.size()) {
                    currentStart = i;
                    currentIsRTL = IsRTLCodepoint(codepoints[i]);
                    currentIsDigits = IsDigit(codepoints[i]);
                }
            }
        }

        // Step 3: Reverse segment order if overall RTL and mixed text
        if (overallRTL && segments.size() > 1) {
            std::reverse(segments.begin(), segments.end());
        }

        // Step 4: Shape each segment
        float currentX = 0.0f;

        for (const auto& segment : segments) {
            if (segment.text.empty()) continue;

            hb_buffer_reset(m_HBBuffer);

            // ========================================
            // NEW: Digits are ALWAYS shaped as LTR
            // ========================================
            if (segment.isDigits) {
                hb_buffer_set_direction(m_HBBuffer, HB_DIRECTION_LTR);
                hb_buffer_set_script(m_HBBuffer, HB_SCRIPT_LATIN);
                hb_buffer_set_language(m_HBBuffer, hb_language_from_string("en", -1));
            }
            else if (segment.isRTL) {
                hb_buffer_set_direction(m_HBBuffer, HB_DIRECTION_RTL);
                hb_buffer_set_script(m_HBBuffer, HB_SCRIPT_ARABIC);
                hb_buffer_set_language(m_HBBuffer, hb_language_from_string("ar", -1));
            }
            else {
                hb_buffer_set_direction(m_HBBuffer, HB_DIRECTION_LTR);
                hb_buffer_set_script(m_HBBuffer, HB_SCRIPT_LATIN);
                hb_buffer_set_language(m_HBBuffer, hb_language_from_string("en", -1));
            }

            // Add text
            hb_buffer_add_utf8(m_HBBuffer, segment.text.c_str(), segment.text.length(), 0, segment.text.length());

            // Shape
            hb_shape(m_HBFont, m_HBBuffer, nullptr, 0);

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

        return glm::vec2(width, height);
    }

} // namespace Unicorn::UI