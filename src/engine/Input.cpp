#include "Input.h"
#include <SDL.h>
#include <algorithm>

Input::Input() {
    m_MousePosition = Vec2(0, 0);
    m_LastMousePosition = Vec2(0, 0);
    m_MouseDelta = Vec2(0, 0);
}

Input::~Input() {
}

void Input::ProcessEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            if (!event.key.repeat) {
                m_KeysDown[event.key.keysym.scancode] = true;
                m_KeysPressedThisFrame[event.key.keysym.scancode] = true;
            }
            break;

        case SDL_KEYUP:
            m_KeysDown[event.key.keysym.scancode] = false;
            m_KeysReleasedThisFrame[event.key.keysym.scancode] = true;
            break;

        case SDL_MOUSEMOTION:
            m_MousePosition.x = static_cast<f32>(event.motion.x);
            m_MousePosition.y = static_cast<f32>(event.motion.y);
            break;

        case SDL_MOUSEBUTTONDOWN:
            m_MouseButtonsDown[event.button.button] = true;
            m_MouseButtonsPressedThisFrame[event.button.button] = true;
            break;

        case SDL_MOUSEBUTTONUP:
            m_MouseButtonsDown[event.button.button] = false;
            m_MouseButtonsReleasedThisFrame[event.button.button] = true;
            break;

        case SDL_MOUSEWHEEL:
            m_MouseWheel = static_cast<f32>(event.wheel.y);
            break;

        case SDL_FINGERDOWN:
        case SDL_FINGERMOTION:
        case SDL_FINGERUP: {
            // Handle touch for mobile
            Touch touch;
            touch.id = static_cast<i32>(event.tfinger.fingerId);
            // FIX: Scale normalized touch coordinates (0.0 to 1.0) to window pixels
            touch.position = Vec2(
                event.tfinger.x * m_WindowWidth,
                event.tfinger.y * m_WindowHeight
            );

            // Find or add touch
            bool found = false;
            for (auto& t : m_Touches) {
                if (t.id == touch.id) {
                    touch.delta = touch.position - t.position;
                    t = touch;
                    found = true;
                    break;
                }
            }

            if (!found && event.type == SDL_FINGERDOWN) {
                touch.delta = Vec2(0, 0);
                m_Touches.push_back(touch);
            } else if (event.type == SDL_FINGERUP) {
                m_Touches.erase(
                    std::remove_if(m_Touches.begin(), m_Touches.end(),
                        [&](const Touch& t) { return t.id == touch.id; }),
                    m_Touches.end()
                );
            }
            break;
        }
    }
}

void Input::Update() {
    // Clear per-frame state
    m_KeysPressedThisFrame.clear();
    m_KeysReleasedThisFrame.clear();
    m_MouseButtonsPressedThisFrame.clear();
    m_MouseButtonsReleasedThisFrame.clear();
    m_MouseWheel = 0.0f;

    // Update mouse delta
    m_MouseDelta = m_MousePosition - m_LastMousePosition;
    m_LastMousePosition = m_MousePosition;
}

bool Input::IsKeyDown(int scancode) const {
    auto it = m_KeysDown.find(scancode);
    return it != m_KeysDown.end() && it->second;
}

bool Input::IsKeyPressed(int scancode) const {
    auto it = m_KeysPressedThisFrame.find(scancode);
    return it != m_KeysPressedThisFrame.end() && it->second;
}

bool Input::IsKeyReleased(int scancode) const {
    auto it = m_KeysReleasedThisFrame.find(scancode);
    return it != m_KeysReleasedThisFrame.end() && it->second;
}

bool Input::IsMouseButtonDown(MouseButton button) const {
    auto it = m_MouseButtonsDown.find(static_cast<int>(button));
    return it != m_MouseButtonsDown.end() && it->second;
}

bool Input::IsMouseButtonPressed(MouseButton button) const {
    auto it = m_MouseButtonsPressedThisFrame.find(static_cast<int>(button));
    return it != m_MouseButtonsPressedThisFrame.end() && it->second;
}

bool Input::IsMouseButtonReleased(MouseButton button) const {
    auto it = m_MouseButtonsReleasedThisFrame.find(static_cast<int>(button));
    return it != m_MouseButtonsReleasedThisFrame.end() && it->second;
}
