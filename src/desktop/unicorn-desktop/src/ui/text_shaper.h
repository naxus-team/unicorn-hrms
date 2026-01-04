#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

// Forward declarations
typedef struct hb_buffer_t hb_buffer_t;
typedef struct hb_font_t hb_font_t;
typedef struct FT_FaceRec_* FT_Face;

namespace Unicorn::UI {

    // Shaped glyph information
    struct ShapedGlyph {
        uint32_t glyphIndex;    // FreeType glyph index
        uint32_t codepoint;     // Unicode codepoint
        glm::vec2 offset;       // X/Y offset
        glm::vec2 advance;      // X/Y advance
    };

    class TextShaper {
    public:
        enum class TextDirection {
            Auto,  // Detect automatically
            LTR,   // Force left-to-right
            RTL    // Force right-to-left
        };

        TextShaper();
        ~TextShaper();

        bool Init();
        void Shutdown();

        // Set the font face for shaping
        bool SetFont(FT_Face face);

        // Set text direction mode
        void SetDirection(TextDirection dir) { m_Direction = dir; }
        TextDirection GetDirection() const { return m_Direction; }

        // Shape UTF-8 text and return shaped glyphs
        std::vector<ShapedGlyph> ShapeText(const std::string& utf8Text);

        // Calculate text size with proper shaping
        glm::vec2 CalculateTextSize(const std::string& utf8Text);

    private:
        hb_font_t* m_HBFont = nullptr;
        hb_buffer_t* m_HBBuffer = nullptr;
        FT_Face m_Face = nullptr;
        TextDirection m_Direction = TextDirection::Auto;
    };

} // namespace Unicorn::UI