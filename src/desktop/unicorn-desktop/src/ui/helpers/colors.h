#pragma once

#include "ui_context.h"
#include <glm/vec4.hpp>

namespace Unicorn::UI {

    // Color definitions
    const glm::vec4 Color::Transparent = { 1.0f, 1.0f, 1.0f, 0.0f };
    const glm::vec4 Color::White = { 1.0f, 1.0f, 1.0f, 1.0f };
    const glm::vec4 Color::Black = { 0.0f, 0.0f, 0.0f, 1.0f };
    const glm::vec4 Color::Primary = { 0.26f, 0.59f, 0.98f, 1.0f };
    const glm::vec4 Color::Secondary = { 0.6f, 0.2f, 0.8f, 1.0f };
    const glm::vec4 Color::Background = { 0.12f, 0.12f, 0.12f, 1.0f };
    const glm::vec4 Color::Panel = { 0.0f, 0.0f, 0.0f, 0.2f };
    const glm::vec4 Color::Border = { 0.0f, 0.0f, 0.0f, 0.08f };
    const glm::vec4 Color::Text = { 0.0f, 0.0f, 0.0f, 1.0f };
    const glm::vec4 Color::TextDisabled = { 0.5f, 0.5f, 0.5f, 1.0f };
    const glm::vec4 Color::TextSecondary = { 0.4f, 0.4f, 0.4f, 1.0f };
    const glm::vec4 Color::ButtonNormal = { 1.0f, 1.0f, 1.0f, 1.0f };
    const glm::vec4 Color::ButtonHover = { 0.92f, 0.92f, 0.92f, 1.0f };
    const glm::vec4 Color::ButtonActive = { 0.9f, 0.9f, 0.9f, 1.0f };

} // namespace Unicorn::UI