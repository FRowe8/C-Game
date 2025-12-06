#pragma once
#include "GameState.h"

class RmlUiSystem;

// Forward declarations
class Renderer;

class UIManager {
public:
    UIManager(GameState* gameState);
    ~UIManager() = default;

    void Initialize();
    void Render(Renderer* renderer);

    // Attach the active RmlUiSystem instance so the manager can bind data to RML documents.
    void SetRmlSystem(RmlUiSystem* system) { m_RmlSystem = system; }

private:
    GameState* m_GameState;
    RmlUiSystem* m_RmlSystem = nullptr;
    ActiveModal m_LastModal = ActiveModal::None;

    void SyncResources();
    void SyncActiveView();
    void SyncPanels(Renderer* renderer);
    std::string FormatValue(double value) const;
};