#pragma once

#include "Types.h"
#include <unordered_map>
#include <vector>

union SDL_Event;

enum class MouseButton {
    Left = 1,
    Middle = 2,
    Right = 3
};

class Input {
public:
    Input();
    ~Input();

    void ProcessEvent(const SDL_Event& event);
    void Update();

    // Keyboard
    bool IsKeyDown(int scancode) const;
    bool IsKeyPressed(int scancode) const;
    bool IsKeyReleased(int scancode) const;

    // Mouse
    Vec2 GetMousePosition() const { return m_MousePosition; }
    Vec2 GetMouseDelta() const { return m_MouseDelta; }

    bool IsMouseButtonDown(MouseButton button) const;
    bool IsMouseButtonPressed(MouseButton button) const;
    bool IsMouseButtonReleased(MouseButton button) const;

    f32 GetMouseWheel() const { return m_MouseWheel; }

    // Touch (for mobile)
    struct Touch {
        i32 id;
        Vec2 position;
        Vec2 delta;
    };

    const std::vector<Touch>& GetTouches() const { return m_Touches; }

    // Window size (needed for touch coordinate scaling)
    void SetWindowSize(f32 width, f32 height) {
        m_WindowWidth = width;
        m_WindowHeight = height;
    }

    // ADDED PUBLIC ACCESSORS to fix compilation errors
    inline f32 GetWindowWidth() const { return m_WindowWidth; }
    inline f32 GetWindowHeight() const { return m_WindowHeight; }

private:
    // Keyboard state
    std::unordered_map<int, bool> m_KeysDown;
    std::unordered_map<int, bool> m_KeysPressedThisFrame;
    std::unordered_map<int, bool> m_KeysReleasedThisFrame;

    // Mouse state
    Vec2 m_MousePosition;
    Vec2 m_LastMousePosition;
    Vec2 m_MouseDelta;
    f32 m_MouseWheel = 0.0f;

    std::unordered_map<int, bool> m_MouseButtonsDown;
    std::unordered_map<int, bool> m_MouseButtonsPressedThisFrame;
    std::unordered_map<int, bool> m_MouseButtonsReleasedThisFrame;

    // Touch state
    std::vector<Touch> m_Touches;

    // Window size (needed for touch coordinate scaling)
    f32 m_WindowWidth = 0.0f;
    f32 m_WindowHeight = 0.0f;
};