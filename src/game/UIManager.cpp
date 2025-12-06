#include "UIManager.h"
#include "GameUtils.h"
#include "RmlUiSystem.h"
#include <string>

UIManager::UIManager(GameState* state) : m_GameState(state) {}

void UIManager::Initialize() {
    // All styling now lives in RCSS; no ImGui style configuration required.
}

void UIManager::Render(Renderer* renderer) {
    (void)renderer;

    // When running without the RmlUi backend there is nothing to render.
    if (!m_RmlSystem || !m_RmlSystem->IsInitialized() || !m_GameState) {
        return;
    }

    // Keep HUD resource counters in sync with the current game state.
    SyncResources();

    // Activate the correct view panel and refresh any data-driven content.
    SyncActiveView();
    SyncPanels(renderer);
}

void UIManager::SyncResources() {
    // Raw values
    m_RmlSystem->UpdateUIResource("qubits", m_GameState->GetResource(QuantumResource::Qubits));
    m_RmlSystem->UpdateUIResource("coherence", m_GameState->GetCoherence());
    m_RmlSystem->UpdateUIResource("entanglement", m_GameState->GetResource(QuantumResource::Entanglement));
    m_RmlSystem->UpdateUIResource("photons", static_cast<f64>(m_GameState->GetTimeline().photons));
    m_RmlSystem->UpdateUIResource("singularities", static_cast<f64>(m_GameState->GetTimeline().singularities));

    // Formatted labels for the HUD chips
    m_RmlSystem->UpdateUIResourceFormatted("qubits", FormatValue(m_GameState->GetResource(QuantumResource::Qubits)));
    m_RmlSystem->UpdateUIResourceFormatted("coherence", FormatValue(m_GameState->GetCoherence()));
    m_RmlSystem->UpdateUIResourceFormatted("entanglement", FormatValue(m_GameState->GetResource(QuantumResource::Entanglement)));
    m_RmlSystem->UpdateUIResourceFormatted("photons", FormatValue(static_cast<f64>(m_GameState->GetTimeline().photons)));
    m_RmlSystem->UpdateUIResourceFormatted("singularities", FormatValue(static_cast<f64>(m_GameState->GetTimeline().singularities)));

    // Coherence progress bar and percentage text
    double maxCoherence = m_GameState->GetMaxCoherence();
    double coherence = m_GameState->GetCoherence();
    float coherenceRatio = (maxCoherence > 0.0) ? static_cast<float>(coherence / maxCoherence) : 0.0f;
    float coherencePercent = coherenceRatio * 100.0f;

    m_RmlSystem->UpdateUIProgress("coherence-progress", coherenceRatio);
    m_RmlSystem->UpdateUIResourceFormatted("coherence-percent", std::to_string(static_cast<int>(coherencePercent)) + "%");
}

void UIManager::SyncActiveView() {
    ActiveModal activeModal = m_GameState->GetActiveModal();
    if (activeModal == m_LastModal) {
        return;
    }

    m_LastModal = activeModal;

    auto setView = [&](const std::string& viewId) {
        if (!viewId.empty()) {
            m_RmlSystem->ActivateView(viewId);
        }
    };

    switch (activeModal) {
        case ActiveModal::None:
            setView("view-stations");
            break;
        case ActiveModal::Research:
            setView("view-research");
            break;
        case ActiveModal::Buyables:
            setView("view-upgrades");
            break;
        case ActiveModal::Combat:
            setView("view-combat");
            break;
        case ActiveModal::Spaceship:
            setView("view-collection");
            break;
        case ActiveModal::Skills:
        case ActiveModal::SpecializedSkills:
            setView("view-collection");
            break;
        case ActiveModal::Gatcha:
            setView("view-collection");
            break;
        case ActiveModal::Achievements:
        case ActiveModal::Statistics:
        case ActiveModal::Milestones:
        case ActiveModal::Challenges:
        case ActiveModal::EssenceShop:
        case ActiveModal::SingularityShop:
        case ActiveModal::Enhancement:
        case ActiveModal::MoreMenu:
        default:
            setView("view-menu");
            break;
    }
}

void UIManager::SyncPanels(Renderer* renderer) {
    (void)renderer;

    // Update view-specific panels so dynamic content matches the selected modal.
    switch (m_LastModal) {
        case ActiveModal::None:
            m_RmlSystem->UpdateStations(m_GameState);
            break;
        case ActiveModal::Research:
            m_RmlSystem->UpdateResearch(m_GameState);
            break;
        case ActiveModal::Buyables:
            m_RmlSystem->UpdateBuyables(m_GameState);
            break;
        case ActiveModal::Combat:
            m_RmlSystem->UpdateCombat(m_GameState);
            break;
        case ActiveModal::Spaceship:
            m_RmlSystem->UpdateSpaceship(m_GameState);
            break;
        case ActiveModal::Skills:
        case ActiveModal::SpecializedSkills:
            m_RmlSystem->UpdateCollection(m_GameState);
            break;
        case ActiveModal::Gatcha:
            m_RmlSystem->UpdateCollection(m_GameState);
            break;
        case ActiveModal::Achievements:
            m_RmlSystem->UpdateAchievements(m_GameState);
            break;
        case ActiveModal::Statistics:
            m_RmlSystem->UpdateStatistics(m_GameState);
            break;
        case ActiveModal::Milestones:
        case ActiveModal::Challenges:
        case ActiveModal::EssenceShop:
        case ActiveModal::SingularityShop:
        case ActiveModal::Enhancement:
        case ActiveModal::MoreMenu:
        default:
            m_RmlSystem->UpdateMenu(m_GameState);
            break;
    }
}

std::string UIManager::FormatValue(double value) const {
    return GameUtils::FormatNumber(value, m_GameState->m_NumberFormat);
}
