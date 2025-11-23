#include "GameState.h"
#include "Renderer.h"
#include "Input.h"
#include "Logger.h"
#include "Platform.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

// ResearchStation implementation
ResearchStation::ResearchStation()
    : baseProduction(0), currentProduction(0), level(0),
      upgradeCost(0), upgradeCostMultiplier(1.15f),
      superpositionValue(0), superpositionProbability(0.5f),
      unlocked(false), unlockCost(0) {
}

void ResearchStation::Upgrade() {
    if (level == 0) {
        // First purchase - unlock
        unlocked = true;
        level = 1;
        currentProduction = baseProduction;
    } else {
        level++;
        currentProduction = baseProduction * level;
    }

    upgradeCost = upgradeCost * upgradeCostMultiplier;
}

void ResearchStation::Observe(GameState* state) {
    if (superpositionValue <= 0) return;

    // Quantum observation - collapse the wave function!
    f64 roll = static_cast<f64>(rand()) / RAND_MAX;
    f64 collapsedValue = superpositionValue;

    if (roll < superpositionProbability) {
        // Success - full value
        collapsedValue *= 1.0;
    } else {
        // Partial collapse
        collapsedValue *= (0.5 + roll * 0.5);
    }

    state->AddResource(resourceType, collapsedValue);
    superpositionValue = 0;

    // Spawn particles
    // (Will be called from GameState with renderer access)
}

void ResearchStation::Update(f64 deltaTime) {
    if (!unlocked || level == 0) return;

    // Accumulate in superposition
    superpositionValue += currentProduction * deltaTime;
}

// UIButton implementation
void UIButton::Update(const Vec2& mousePos) {
    hovered = enabled && bounds.Contains(mousePos);
}

void UIButton::Render(Renderer* renderer) {
    Color renderColor = enabled ? (hovered ? hoverColor : color) : Color(0.3f, 0.3f, 0.3f, 0.5f);

    renderer->DrawRect(bounds, renderColor, true);
    renderer->DrawRect(bounds, Color::White() * 0.8f, false);

    // Draw text (centered)
    Vec2 textPos = bounds.Center();
    textPos.x -= text.length() * 5.0f; // Rough centering
    textPos.y -= 8.0f;

    Color textColor = enabled ? Color::White() : Color(0.6f, 0.6f, 0.6f, 1.0f);
    renderer->DrawText(text, textPos, textColor, 16.0f);
}

bool UIButton::WasClicked(const Vec2& mousePos, bool mousePressed) {
    if (!enabled) return false;
    if (!bounds.Contains(mousePos)) return false;
    return mousePressed;
}

// GameState implementation
GameState::GameState()
    : m_TotalTimePlayed(0), m_TimeSinceLastSave(0),
      m_Coherence(100), m_MaxCoherence(100), m_CoherenceDecayRate(1.0) {

    for (int i = 0; i < 3; i++) {
        m_Resources[i] = 0;
    }

    m_Timeline.completedResets = 0;
    m_Timeline.photons = 0;
    m_Timeline.photonBonus = 1.0;
}

GameState::~GameState() {
}

void GameState::Initialize() {
    Log::Info("Initializing game state...");

    InitializeStations();
    InitializeUI();

    // Give starting resources
    m_Resources[static_cast<int>(QuantumResource::Qubits)] = 10.0;
    m_Resources[static_cast<int>(QuantumResource::Coherence)] = 50.0;
    m_Resources[static_cast<int>(QuantumResource::Entanglement)] = 0.0;

    // Try to load save file
    std::string savePath = Platform::GetSaveDirectory() + "quantum_save.json";
    if (Platform::FileExists(savePath)) {
        if (Load(savePath)) {
            Log::Info("Save file loaded successfully");
        }
    }

    Log::Info("Game state initialized");
}

void GameState::InitializeStations() {
    // Station 1: Basic Qubit Generator
    ResearchStation station1;
    station1.name = "Qubit Generator";
    station1.description = "Generates qubits in superposition";
    station1.resourceType = QuantumResource::Qubits;
    station1.baseProduction = 1.0;
    station1.currentProduction = 0;
    station1.level = 0;
    station1.upgradeCost = 10.0;
    station1.upgradeCostMultiplier = 1.15;
    station1.superpositionProbability = 0.7;
    station1.unlocked = true; // First one is unlocked
    m_Stations.push_back(station1);

    // Station 2: Coherence Stabilizer
    ResearchStation station2;
    station2.name = "Coherence Stabilizer";
    station2.description = "Maintains quantum coherence";
    station2.resourceType = QuantumResource::Coherence;
    station2.baseProduction = 0.5;
    station2.level = 0;
    station2.upgradeCost = 25.0;
    station2.upgradeCostMultiplier = 1.18;
    station2.superpositionProbability = 0.8;
    station2.unlocked = false;
    station2.unlockCost = 50.0;
    m_Stations.push_back(station2);

    // Station 3: Entanglement Chamber
    ResearchStation station3;
    station3.name = "Entanglement Chamber";
    station3.description = "Creates quantum entanglement";
    station3.resourceType = QuantumResource::Entanglement;
    station3.baseProduction = 0.2;
    station3.level = 0;
    station3.upgradeCost = 100.0;
    station3.upgradeCostMultiplier = 1.2;
    station3.superpositionProbability = 0.6;
    station3.unlocked = false;
    station3.unlockCost = 150.0;
    m_Stations.push_back(station3);

    // Station 4: Advanced Qubit Synthesizer
    ResearchStation station4;
    station4.name = "Qubit Synthesizer";
    station4.description = "Advanced qubit generation";
    station4.resourceType = QuantumResource::Qubits;
    station4.baseProduction = 5.0;
    station4.level = 0;
    station4.upgradeCost = 500.0;
    station4.upgradeCostMultiplier = 1.25;
    station4.superpositionProbability = 0.5;
    station4.unlocked = false;
    station4.unlockCost = 300.0;
    m_Stations.push_back(station4);

    // Station 5: Quantum Supercomputer
    ResearchStation station5;
    station5.name = "Quantum Supercomputer";
    station5.description = "Massive parallel processing";
    station5.resourceType = QuantumResource::Qubits;
    station5.baseProduction = 20.0;
    station5.level = 0;
    station5.upgradeCost = 2000.0;
    station5.upgradeCostMultiplier = 1.3;
    station5.superpositionProbability = 0.4;
    station5.unlocked = false;
    station5.unlockCost = 1000.0;
    m_Stations.push_back(station5);
}

void GameState::InitializeUI() {
    m_ScrollOffset = Vec2(0, 0);
    // UI buttons will be created dynamically during rendering
}

void GameState::Update(f64 deltaTime, Input* input, Renderer* renderer) {
    m_TotalTimePlayed += deltaTime;
    m_TimeSinceLastSave += deltaTime;

    // Update stations
    UpdateStations(deltaTime);

    // Update coherence
    UpdateCoherence(deltaTime);

    // Update UI
    UpdateUI(input);

    // Auto-save every 30 seconds
    if (m_TimeSinceLastSave >= 30.0) {
        std::string savePath = Platform::GetSaveDirectory() + "quantum_save.json";
        Save(savePath);
        m_TimeSinceLastSave = 0;
    }
}

void GameState::UpdateStations(f64 deltaTime) {
    // Apply prestige bonus
    f64 globalMultiplier = m_Timeline.photonBonus;

    for (auto& station : m_Stations) {
        station.Update(deltaTime * globalMultiplier);
    }
}

void GameState::UpdateCoherence(f64 deltaTime) {
    // Coherence slowly decays
    m_Coherence -= m_CoherenceDecayRate * deltaTime;
    if (m_Coherence < 0) m_Coherence = 0;

    // Coherence affects production
    f64 coherenceMultiplier = m_Coherence / m_MaxCoherence;
    // Apply to stations (already done in UpdateStations)
}

void GameState::UpdateUI(Input* input) {
    Vec2 mousePos = input->GetMousePosition();
    bool mousePressed = input->IsMouseButtonPressed(MouseButton::Left);

    // Update buttons
    for (auto& button : m_Buttons) {
        button.Update(mousePos);

        if (button.WasClicked(mousePos, mousePressed) && button.onClick) {
            button.onClick();
        }
    }
}

void GameState::Render(Renderer* renderer) {
    RenderResources(renderer);
    RenderStations(renderer);
    RenderUI(renderer);
}

void GameState::RenderResources(Renderer* renderer) {
    // Draw resource panel at top
    f32 panelHeight = 100.0f;
    Rect panel(0, 0, static_cast<f32>(renderer->GetWidth()), panelHeight);
    renderer->DrawRect(panel, Color(0.1f, 0.1f, 0.15f, 0.9f), true);

    const char* resourceNames[] = {"Qubits", "Coherence", "Entanglement"};
    Color resourceColors[] = {
        Color::QuantumBlue(),
        Color::CoherenceGreen(),
        Color::EntanglementOrange()
    };

    f32 xOffset = 20.0f;
    for (int i = 0; i < 3; i++) {
        Vec2 textPos(xOffset, 20.0f);

        renderer->DrawText(resourceNames[i], textPos, Color::White(), 16.0f);

        // Draw value
        std::ostringstream oss;
        oss.precision(1);
        oss << std::fixed << m_Resources[i];
        Vec2 valuePos(xOffset, 50.0f);
        renderer->DrawText(oss.str(), valuePos, resourceColors[i], 24.0f);

        xOffset += 250.0f;
    }

    // Draw coherence bar
    f32 coherenceBarWidth = 200.0f;
    f32 coherenceBarHeight = 20.0f;
    Vec2 coherenceBarPos(static_cast<f32>(renderer->GetWidth()) - coherenceBarWidth - 20.0f, 40.0f);

    renderer->DrawText("Coherence", Vec2(coherenceBarPos.x, coherenceBarPos.y - 20.0f), Color::White(), 14.0f);

    Rect coherenceBarBg(coherenceBarPos.x, coherenceBarPos.y, coherenceBarWidth, coherenceBarHeight);
    renderer->DrawRect(coherenceBarBg, Color(0.2f, 0.2f, 0.2f, 1.0f), true);

    f32 coherenceFill = (m_Coherence / m_MaxCoherence) * coherenceBarWidth;
    Rect coherenceBarFill(coherenceBarPos.x, coherenceBarPos.y, coherenceFill, coherenceBarHeight);
    renderer->DrawRect(coherenceBarFill, Color::CoherenceGreen(), true);

    renderer->DrawRect(coherenceBarBg, Color::White() * 0.5f, false);
}

void GameState::RenderStations(Renderer* renderer) {
    m_Buttons.clear(); // Rebuild buttons each frame for simplicity

    f32 startY = 120.0f;
    f32 stationHeight = 120.0f;
    f32 margin = 10.0f;

    for (size_t i = 0; i < m_Stations.size(); i++) {
        auto& station = m_Stations[i];
        f32 y = startY + i * (stationHeight + margin) + m_ScrollOffset.y;

        // Station background
        Rect stationRect(20.0f, y, static_cast<f32>(renderer->GetWidth()) - 40.0f, stationHeight);

        Color bgColor = station.unlocked ? Color(0.15f, 0.15f, 0.2f, 0.9f) : Color(0.1f, 0.1f, 0.1f, 0.5f);
        renderer->DrawRect(stationRect, bgColor, true);
        renderer->DrawRect(stationRect, Color::White() * 0.3f, false);

        if (!station.unlocked) {
            // Draw unlock button
            Vec2 titlePos(40.0f, y + 20.0f);
            renderer->DrawText(station.name + " (LOCKED)", titlePos, Color(0.5f, 0.5f, 0.5f, 1.0f), 20.0f);

            UIButton unlockBtn;
            unlockBtn.bounds = Rect(40.0f, y + 60.0f, 200.0f, 40.0f);
            unlockBtn.text = "Unlock (" + std::to_string(static_cast<int>(station.unlockCost)) + " Qubits)";
            unlockBtn.color = Color::QuantumBlue() * 0.7f;
            unlockBtn.hoverColor = Color::QuantumBlue();
            unlockBtn.enabled = m_Resources[0] >= station.unlockCost;
            unlockBtn.onClick = [this, i]() {
                if (SpendResource(QuantumResource::Qubits, m_Stations[i].unlockCost)) {
                    m_Stations[i].unlocked = true;
                    m_Stations[i].level = 0; // Not upgraded yet
                    Log::Infof("Unlocked: ", m_Stations[i].name);
                }
            };

            unlockBtn.Render(renderer);
            m_Buttons.push_back(unlockBtn);
            continue;
        }

        // Draw station info
        Vec2 titlePos(40.0f, y + 10.0f);
        renderer->DrawText(station.name + " Lv." + std::to_string(station.level), titlePos, Color::White(), 18.0f);

        Vec2 descPos(40.0f, y + 35.0f);
        renderer->DrawText(station.description, descPos, Color(0.7f, 0.7f, 0.7f, 1.0f), 12.0f);

        // Draw production
        std::ostringstream prodOss;
        prodOss.precision(2);
        prodOss << std::fixed << station.currentProduction << "/s";
        Vec2 prodPos(40.0f, y + 55.0f);
        renderer->DrawText("Production: " + prodOss.str(), prodPos, Color::CoherenceGreen(), 14.0f);

        // Draw superposition value
        std::ostringstream superOss;
        superOss.precision(1);
        superOss << std::fixed << station.superpositionValue;
        Vec2 superPos(300.0f, y + 55.0f);
        renderer->DrawText("Superposition: " + superOss.str(), superPos, Color::QuantumPurple(), 14.0f);

        // Draw probability
        std::ostringstream probOss;
        probOss.precision(0);
        probOss << std::fixed << (station.superpositionProbability * 100.0f) << "%";
        Vec2 probPos(550.0f, y + 55.0f);
        renderer->DrawText("Success: " + probOss.str(), probPos, Color::Yellow(), 14.0f);

        // Observe button
        UIButton observeBtn;
        observeBtn.bounds = Rect(40.0f, y + 75.0f, 150.0f, 35.0f);
        observeBtn.text = "OBSERVE";
        observeBtn.color = Color::QuantumPurple() * 0.7f;
        observeBtn.hoverColor = Color::QuantumPurple();
        observeBtn.enabled = station.superpositionValue > 0.1;
        observeBtn.onClick = [this, i, renderer]() {
            auto& st = m_Stations[i];
            f64 value = st.superpositionValue;
            st.Observe(this);

            // Spawn particle effects
            Rect stationRect(20.0f, 120.0f + i * 130.0f, 800.0f, 120.0f);
            Vec2 center = stationRect.Center();
            for (int p = 0; p < 20; p++) {
                Renderer::Particle particle;
                particle.position = center;
                f32 angle = (rand() % 360) * 3.14159f / 180.0f;
                f32 speed = 50.0f + (rand() % 100);
                particle.velocity = Vec2(cosf(angle) * speed, sinf(angle) * speed);
                particle.color = Color::QuantumPurple();
                particle.life = 0.5f + (rand() % 100) / 200.0f;
                particle.maxLife = particle.life;
                particle.size = 3.0f + (rand() % 5);
                renderer->AddParticle(particle);
            }

            Log::Infof("Observed ", st.name, ", collapsed value: ", value);
        };

        observeBtn.Render(renderer);
        m_Buttons.push_back(observeBtn);

        // Upgrade button
        UIButton upgradeBtn;
        upgradeBtn.bounds = Rect(200.0f, y + 75.0f, 200.0f, 35.0f);
        upgradeBtn.text = "Upgrade (" + std::to_string(static_cast<int>(station.upgradeCost)) + ")";
        upgradeBtn.color = Color::EntanglementOrange() * 0.7f;
        upgradeBtn.hoverColor = Color::EntanglementOrange();
        upgradeBtn.enabled = m_Resources[0] >= station.upgradeCost;
        upgradeBtn.onClick = [this, i]() {
            if (SpendResource(QuantumResource::Qubits, m_Stations[i].upgradeCost)) {
                m_Stations[i].Upgrade();
                Log::Infof("Upgraded ", m_Stations[i].name, " to level ", m_Stations[i].level);
            }
        };

        upgradeBtn.Render(renderer);
        m_Buttons.push_back(upgradeBtn);
    }

    // Prestige button
    f32 prestigeY = startY + m_Stations.size() * (stationHeight + margin) + 20.0f + m_ScrollOffset.y;
    f64 photonsOnPrestige = CalculatePhotonsOnPrestige();

    UIButton prestigeBtn;
    prestigeBtn.bounds = Rect(20.0f, prestigeY, 400.0f, 60.0f);
    prestigeBtn.text = "PRESTIGE (+" + std::to_string(static_cast<int>(photonsOnPrestige)) + " Photons)";
    prestigeBtn.color = Color::Magenta() * 0.5f;
    prestigeBtn.hoverColor = Color::Magenta();
    prestigeBtn.enabled = photonsOnPrestige > 0;
    prestigeBtn.onClick = [this]() {
        PerformPrestige();
    };

    prestigeBtn.Render(renderer);
    m_Buttons.push_back(prestigeBtn);
}

void GameState::RenderUI(Renderer* renderer) {
    // Additional UI elements can go here
    // Prestige info, achievements, etc.
}

void GameState::AddResource(QuantumResource type, f64 amount) {
    m_Resources[static_cast<int>(type)] += amount;
}

bool GameState::SpendResource(QuantumResource type, f64 amount) {
    int idx = static_cast<int>(type);
    if (m_Resources[idx] >= amount) {
        m_Resources[idx] -= amount;
        return true;
    }
    return false;
}

f64 GameState::GetResource(QuantumResource type) const {
    return m_Resources[static_cast<int>(type)];
}

f64 GameState::CalculatePhotonsOnPrestige() const {
    // Photons based on total qubits earned
    f64 totalQubits = m_Resources[0];
    if (totalQubits < 1000.0) return 0;

    return std::floor(std::sqrt(totalQubits / 100.0));
}

void GameState::PerformPrestige() {
    f64 photons = CalculatePhotonsOnPrestige();
    if (photons <= 0) {
        Log::Warning("Not enough progress for prestige");
        return;
    }

    Log::Infof("Performing prestige! Gained ", photons, " photons");

    m_Timeline.photons += photons;
    m_Timeline.completedResets++;
    m_Timeline.photonBonus = 1.0 + (m_Timeline.photons * 0.1); // 10% per photon

    // Reset resources
    for (int i = 0; i < 3; i++) {
        m_Resources[i] = 0;
    }
    m_Resources[0] = 10.0; // Start with 10 qubits

    // Reset stations
    for (auto& station : m_Stations) {
        if (station.name != "Qubit Generator") {
            station.unlocked = false;
        }
        station.level = 0;
        station.currentProduction = 0;
        station.superpositionValue = 0;
        station.upgradeCost = station.upgradeCostMultiplier; // Reset cost
    }

    m_Coherence = m_MaxCoherence;
}

bool GameState::Save(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        Log::Errorf("Failed to open save file: ", filepath);
        return false;
    }

    // Simple JSON-like format
    file << "{\n";
    file << "  \"version\": 1,\n";
    file << "  \"resources\": [" << m_Resources[0] << ", " << m_Resources[1] << ", " << m_Resources[2] << "],\n";
    file << "  \"coherence\": " << m_Coherence << ",\n";
    file << "  \"photons\": " << m_Timeline.photons << ",\n";
    file << "  \"resets\": " << m_Timeline.completedResets << ",\n";
    file << "  \"timePlayed\": " << m_TotalTimePlayed << ",\n";
    file << "  \"stations\": [\n";

    for (size_t i = 0; i < m_Stations.size(); i++) {
        const auto& s = m_Stations[i];
        file << "    {\"level\": " << s.level << ", \"unlocked\": " << (s.unlocked ? "true" : "false") << "}";
        if (i < m_Stations.size() - 1) file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";

    file.close();
    Log::Debugf("Game saved to ", filepath);
    return true;
}

bool GameState::Load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    // Simple parsing (not a full JSON parser)
    std::string line;
    while (std::getline(file, line)) {
        // This is a very basic parser - in production you'd use a real JSON library
        if (line.find("\"resources\"") != std::string::npos) {
            // Parse resources array
            size_t start = line.find('[');
            size_t end = line.find(']');
            if (start != std::string::npos && end != std::string::npos) {
                std::string values = line.substr(start + 1, end - start - 1);
                std::istringstream iss(values);
                std::string val;
                int idx = 0;
                while (std::getline(iss, val, ',') && idx < 3) {
                    m_Resources[idx++] = std::stod(val);
                }
            }
        }
        else if (line.find("\"photons\"") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                m_Timeline.photons = std::stod(line.substr(pos + 1));
                m_Timeline.photonBonus = 1.0 + (m_Timeline.photons * 0.1);
            }
        }
        // Add more parsing as needed
    }

    file.close();
    Log::Infof("Game loaded from ", filepath);
    return true;
}
