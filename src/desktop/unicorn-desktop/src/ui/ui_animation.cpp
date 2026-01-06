#include "ui_animation.h"
#include <algorithm>

namespace Unicorn::UI {

    void AnimationController::Update(float dt) {
        // Update all button animations
        for (auto& [id, state] : m_ButtonStates) {
            // Smooth decay animations
            if (state.hoverProgress > 0.0f) {
                state.hoverProgress = std::max(0.0f, state.hoverProgress - dt * m_AnimSpeed);
            }
            if (state.activeProgress > 0.0f) {
                state.activeProgress = std::max(0.0f, state.activeProgress - dt * m_AnimSpeed);
            }
        }
    }

    AnimationController::ButtonState& AnimationController::GetButtonState(const std::string& id) {
        return m_ButtonStates[id];
    }

    float AnimationController::Lerp(float a, float b, float t) {
        return a + (b - a) * std::min(1.0f, std::max(0.0f, t));
    }

    glm::vec4 AnimationController::LerpColor(const glm::vec4& a, const glm::vec4& b, float t) {
        return glm::vec4(
            Lerp(a.r, b.r, t),
            Lerp(a.g, b.g, t),
            Lerp(a.b, b.b, t),
            Lerp(a.a, b.a, t)
        );
    }

} // namespace Unicorn::UI