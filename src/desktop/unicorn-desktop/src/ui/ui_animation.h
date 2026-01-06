#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

namespace Unicorn::UI {

    class AnimationController {
    public:
        struct ButtonState {
            float hoverProgress = 0.0f;    // 0 to 1
            float activeProgress = 0.0f;   // 0 to 1
            float scale = 1.0f;
            glm::vec4 color;
        };

        bool HasActiveAnimations() const {
            for (const auto& [id, state] : m_ButtonStates) {
                if (state.hoverProgress > 0.01f ||
                    state.hoverProgress < 0.99f ||
                    state.activeProgress > 0.01f) {
                    return true;
                }
            }
            return false;
        }

        void Update(float dt);

        // Get/Set button state
        ButtonState& GetButtonState(const std::string& id);

        // Smooth interpolation
        static float Lerp(float a, float b, float t);
        static glm::vec4 LerpColor(const glm::vec4& a, const glm::vec4& b, float t);

    private:
        std::unordered_map<std::string, ButtonState> m_ButtonStates;
        const float m_AnimSpeed = 8.0f; // Animation speed
    };

} // namespace Unicorn::UI