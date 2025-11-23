#pragma once

#include "Types.h"
#include <vector>
#include <string>
#include <functional>

// Forward declarations
class Renderer;
class Input;

// Quantum resource types
enum class QuantumResource {
    Qubits,        // Primary currency
    Coherence,     // Stability resource
    Entanglement   // Strategic resource
};

// Research station that generates resources
struct ResearchStation {
    std::string name;
    std::string description;
    QuantumResource resourceType;

    f64 baseProduction;        // Base production per second
    f64 currentProduction;     // Current production (with bonuses)

    i32 level;                 // Upgrade level
    f64 upgradeCost;           // Cost to upgrade
    f64 upgradeCostMultiplier; // How much cost increases per level

    // Superposition state (resources waiting to be observed)
    f64 superpositionValue;
    f64 superpositionProbability; // Chance of getting full value on observation

    bool unlocked;
    f64 unlockCost;

    ResearchStation();
    void Upgrade();
    void Observe(class GameState* state); // Collapse superposition
    void Update(f64 deltaTime);
};

// Entanglement pairs - link stations for bonuses
struct EntanglementPair {
    i32 stationA;
    i32 stationB;
    f64 bonusMultiplier;
    f64 coherenceCost;
    bool active;
};

// Prestige currency and bonuses
struct QuantumTimeline {
    i32 completedResets;
    f64 photons; // Prestige currency
    f64 photonBonus; // Global multiplier from photons

    std::vector<bool> permanentUpgrades;
};

// UI Button
struct UIButton {
    Rect bounds;
    std::string text;
    Color color;
    Color hoverColor;
    std::function<void()> onClick;
    bool enabled = true;
    bool hovered = false;

    void Update(const Vec2& mousePos);
    void Render(Renderer* renderer);
    bool WasClicked(const Vec2& mousePos, bool mousePressed);
};

class GameState {
public:
    GameState();
    ~GameState();

    void Initialize();
    void Update(f64 deltaTime, Input* input, Renderer* renderer);
    void Render(Renderer* renderer);

    // Resource management
    void AddResource(QuantumResource type, f64 amount);
    bool SpendResource(QuantumResource type, f64 amount);
    f64 GetResource(QuantumResource type) const;

    // Save/Load
    bool Save(const std::string& filepath);
    bool Load(const std::string& filepath);

    // Prestige
    void PerformPrestige();
    f64 CalculatePhotonsOnPrestige() const;

private:
    void InitializeStations();
    void InitializeUI();
    void UpdateStations(f64 deltaTime);
    void UpdateCoherence(f64 deltaTime);
    void UpdateUI(Input* input);
    void RenderUI(Renderer* renderer);
    void RenderResources(Renderer* renderer);
    void RenderStations(Renderer* renderer);
    void RenderParticleEffects(Renderer* renderer, f64 deltaTime);

    // Resources
    f64 m_Resources[3]; // Qubits, Coherence, Entanglement

    // Game objects
    std::vector<ResearchStation> m_Stations;
    std::vector<EntanglementPair> m_Entanglements;
    QuantumTimeline m_Timeline;

    // UI
    std::vector<UIButton> m_Buttons;
    Vec2 m_ScrollOffset;

    // Game time
    f64 m_TotalTimePlayed;
    f64 m_TimeSinceLastSave;

    // Coherence decay
    f64 m_Coherence;
    f64 m_MaxCoherence;
    f64 m_CoherenceDecayRate;
};
