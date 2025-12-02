#include "CardManager.h"
#include "GameState.h"
#include "Enemy.h"
#include "logger.h"
#include <algorithm>
#include <random>
#include <fstream>

CardManager::CardManager()
    : m_NextCardID(1), m_NextInstanceID(1),
      m_MaxDeckSize(30), m_MaxHandSize(7),
      m_CurrentEnergy(3), m_MaxEnergy(3),
      m_TotalCardsPlayed(0), m_TotalCardsDrawn(0) {
}

void CardManager::Initialize() {
    // Clear existing data
    m_CardDatabase.clear();
    m_Collection.clear();
    m_DeckInstanceIDs.clear();
    m_HandInstanceIDs.clear();
    m_DrawPileInstanceIDs.clear();
    m_DiscardPileInstanceIDs.clear();

    m_NextCardID = 1;
    m_NextInstanceID = 1;
    m_CurrentEnergy = 3;
    m_MaxEnergy = 3;
    m_TotalCardsPlayed = 0;
    m_TotalCardsDrawn = 0;

    Log::Info("CardManager initialized");
}

void CardManager::Update(f64 deltaTime) {
    // Future: Handle time-based card effects, cooldowns, etc.
    (void)deltaTime;
}

// === Card Database ===

void CardManager::RegisterCard(const Card& card) {
    m_CardDatabase[card.id] = card;
    Log::Infof("Registered card: ", card.name, " (ID: ", card.id, ")");
}

Card* CardManager::GetCardDefinition(i32 cardID) {
    auto it = m_CardDatabase.find(cardID);
    if (it != m_CardDatabase.end()) {
        return &it->second;
    }
    return nullptr;
}

const Card* CardManager::GetCardDefinition(i32 cardID) const {
    auto it = m_CardDatabase.find(cardID);
    if (it != m_CardDatabase.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<const Card*> CardManager::GetAllCards() const {
    std::vector<const Card*> cards;
    for (const auto& pair : m_CardDatabase) {
        cards.push_back(&pair.second);
    }
    return cards;
}

std::vector<const Card*> CardManager::GetCardsByRarity(CardRarity rarity) const {
    std::vector<const Card*> cards;
    for (const auto& pair : m_CardDatabase) {
        if (pair.second.rarity == rarity) {
            cards.push_back(&pair.second);
        }
    }
    return cards;
}

// === Player Collection ===

void CardManager::UnlockCard(i32 cardID) {
    Card* card = GetCardDefinition(cardID);
    if (card) {
        card->unlocked = true;
        Log::Infof("Unlocked card: ", card->name);
    }
}

bool CardManager::IsCardUnlocked(i32 cardID) const {
    const Card* card = GetCardDefinition(cardID);
    return card && card->unlocked;
}

CardInstance* CardManager::AddCardToCollection(i32 cardID) {
    if (!IsCardUnlocked(cardID)) {
        Log::Warningf("Cannot add locked card to collection: ", cardID);
        return nullptr;
    }

    CardInstance* instance = new CardInstance(cardID, m_NextInstanceID++);
    m_Collection.push_back(instance);

    const Card* card = GetCardDefinition(cardID);
    if (card) {
        Log::Infof("Added card to collection: ", card->name, " (Instance ID: ", instance->instanceID, ")");
    }

    return instance;
}

std::vector<CardInstance*> CardManager::GetCollection() {
    return m_Collection;
}

const std::vector<CardInstance*>& CardManager::GetCollection() const {
    return m_Collection;
}

i32 CardManager::GetCollectionSize() const {
    return static_cast<i32>(m_Collection.size());
}

// === Deck Building ===

bool CardManager::AddCardToDeck(i32 instanceID) {
    // Check if deck is full
    if (static_cast<i32>(m_DeckInstanceIDs.size()) >= m_MaxDeckSize) {
        Log::Warning("Deck is full");
        return false;
    }

    // Check if card exists in collection
    CardInstance* instance = GetCardInstance(instanceID);
    if (!instance) {
        Log::Warningf("Card instance not found: ", instanceID);
        return false;
    }

    // Check if card is already in deck
    if (instance->inDeck) {
        Log::Warning("Card is already in deck");
        return false;
    }

    m_DeckInstanceIDs.push_back(instanceID);
    instance->inDeck = true;

    const Card* card = GetCardDefinition(instance->cardID);
    if (card) {
        Log::Infof("Added card to deck: ", card->name);
    }

    return true;
}

bool CardManager::RemoveCardFromDeck(i32 instanceID) {
    auto it = std::find(m_DeckInstanceIDs.begin(), m_DeckInstanceIDs.end(), instanceID);
    if (it == m_DeckInstanceIDs.end()) {
        Log::Warningf("Card not in deck: ", instanceID);
        return false;
    }

    m_DeckInstanceIDs.erase(it);

    CardInstance* instance = GetCardInstance(instanceID);
    if (instance) {
        instance->inDeck = false;
    }

    Log::Infof("Removed card from deck: ", instanceID);
    return true;
}

void CardManager::ClearDeck() {
    for (i32 instanceID : m_DeckInstanceIDs) {
        CardInstance* instance = GetCardInstance(instanceID);
        if (instance) {
            instance->inDeck = false;
        }
    }
    m_DeckInstanceIDs.clear();
    Log::Info("Deck cleared");
}

std::vector<CardInstance*> CardManager::GetDeck() {
    std::vector<CardInstance*> deck;
    for (i32 instanceID : m_DeckInstanceIDs) {
        CardInstance* instance = GetCardInstance(instanceID);
        if (instance) {
            deck.push_back(instance);
        }
    }
    return deck;
}

i32 CardManager::GetDeckSize() const {
    return static_cast<i32>(m_DeckInstanceIDs.size());
}

// === Hand Management ===

void CardManager::StartCombat() {
    // Clear hand and piles
    m_HandInstanceIDs.clear();
    m_DrawPileInstanceIDs.clear();
    m_DiscardPileInstanceIDs.clear();

    // Copy deck to draw pile
    m_DrawPileInstanceIDs = m_DeckInstanceIDs;

    // Shuffle draw pile
    ShuffleDeck();

    // Draw opening hand
    DrawCards(m_MaxHandSize);

    // Reset energy
    m_CurrentEnergy = m_MaxEnergy;

    Log::Infof("Combat started - Drew ", static_cast<i32>(m_HandInstanceIDs.size()), " cards");
}

void CardManager::EndCombat() {
    m_HandInstanceIDs.clear();
    m_DrawPileInstanceIDs.clear();
    m_DiscardPileInstanceIDs.clear();
    m_CurrentEnergy = 0;

    Log::Info("Combat ended - Hand cleared");
}

void CardManager::DrawCard() {
    if (m_HandInstanceIDs.size() >= static_cast<size_t>(m_MaxHandSize)) {
        Log::Warning("Hand is full - cannot draw");
        return;
    }

    // If draw pile is empty, shuffle discard pile back into draw pile
    if (m_DrawPileInstanceIDs.empty()) {
        if (m_DiscardPileInstanceIDs.empty()) {
            Log::Warning("No cards to draw");
            return;
        }

        Log::Info("Draw pile empty - shuffling discard pile");
        m_DrawPileInstanceIDs = m_DiscardPileInstanceIDs;
        m_DiscardPileInstanceIDs.clear();
        ShuffleDeck();
    }

    // Draw top card
    i32 instanceID = m_DrawPileInstanceIDs.back();
    m_DrawPileInstanceIDs.pop_back();
    m_HandInstanceIDs.push_back(instanceID);

    m_TotalCardsDrawn++;

    const CardInstance* instance = GetCardInstance(instanceID);
    if (instance) {
        const Card* card = GetCardDefinition(instance->cardID);
        if (card) {
            Log::Infof("Drew card: ", card->name);
        }
    }
}

void CardManager::DrawCards(i32 count) {
    for (i32 i = 0; i < count; i++) {
        DrawCard();
    }
}

void CardManager::DiscardCard(i32 handIndex) {
    if (handIndex < 0 || handIndex >= static_cast<i32>(m_HandInstanceIDs.size())) {
        Log::Warningf("Invalid hand index: ", handIndex);
        return;
    }

    i32 instanceID = m_HandInstanceIDs[handIndex];
    m_HandInstanceIDs.erase(m_HandInstanceIDs.begin() + handIndex);
    m_DiscardPileInstanceIDs.push_back(instanceID);

    Log::Infof("Discarded card from hand index ", handIndex);
}

void CardManager::DiscardHand() {
    for (i32 instanceID : m_HandInstanceIDs) {
        m_DiscardPileInstanceIDs.push_back(instanceID);
    }
    m_HandInstanceIDs.clear();
    Log::Info("Discarded entire hand");
}

void CardManager::ShuffleDeck() {
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::shuffle(m_DrawPileInstanceIDs.begin(), m_DrawPileInstanceIDs.end(), rng);
    Log::Info("Deck shuffled");
}

std::vector<CardInstance*> CardManager::GetHand() {
    std::vector<CardInstance*> hand;
    for (i32 instanceID : m_HandInstanceIDs) {
        CardInstance* instance = GetCardInstance(instanceID);
        if (instance) {
            hand.push_back(instance);
        }
    }
    return hand;
}

const std::vector<CardInstance*>& CardManager::GetHand() const {
    // This is a bit tricky - we need to return a const reference
    // For now, return empty collection (to be refactored)
    static std::vector<CardInstance*> emptyHand;
    return emptyHand;
}

i32 CardManager::GetHandSize() const {
    return static_cast<i32>(m_HandInstanceIDs.size());
}

// === Playing Cards ===

bool CardManager::CanPlayCard(i32 handIndex, f64 qubits, f64 coherence, i32 energy) const {
    if (handIndex < 0 || handIndex >= static_cast<i32>(m_HandInstanceIDs.size())) {
        return false;
    }

    i32 instanceID = m_HandInstanceIDs[handIndex];
    const CardInstance* instance = GetCardInstance(instanceID);
    if (!instance) return false;

    const Card* card = GetCardDefinition(instance->cardID);
    if (!card) return false;

    return card->CanAfford(qubits, coherence, energy);
}

bool CardManager::PlayCard(i32 handIndex, GameState* gameState, Enemy* target) {
    if (!gameState) {
        Log::Error("Cannot play card: null GameState");
        return false;
    }

    if (handIndex < 0 || handIndex >= static_cast<i32>(m_HandInstanceIDs.size())) {
        Log::Warningf("Invalid hand index: ", handIndex);
        return false;
    }

    i32 instanceID = m_HandInstanceIDs[handIndex];
    CardInstance* instance = GetCardInstance(instanceID);
    if (!instance) {
        Log::Error("Card instance not found");
        return false;
    }

    const Card* card = GetCardDefinition(instance->cardID);
    if (!card) {
        Log::Error("Card definition not found");
        return false;
    }

    // Check if we can afford the card (will be checked against GameState resources)
    // For now, just check energy cost
    if (m_CurrentEnergy < card->energyCost) {
        Log::Warningf("Not enough energy to play card (need ", card->energyCost, ", have ", m_CurrentEnergy, ")");
        return false;
    }

    // Spend energy
    m_CurrentEnergy -= card->energyCost;

    // Apply card effects
    for (const auto& effect : card->effects) {
        ApplyCardEffect(effect, gameState, target);
    }

    // Move card from hand to discard pile
    m_HandInstanceIDs.erase(m_HandInstanceIDs.begin() + handIndex);
    m_DiscardPileInstanceIDs.push_back(instanceID);

    m_TotalCardsPlayed++;

    Log::Infof("Played card: ", card->name);
    return true;
}

// === Card Acquisition ===

CardInstance* CardManager::AwardRandomCard(CardRarity minRarity) {
    // Get all unlocked cards
    std::vector<i32> eligibleCardIDs;
    for (const auto& pair : m_CardDatabase) {
        const Card& card = pair.second;
        if (card.unlocked && card.rarity >= minRarity) {
            eligibleCardIDs.push_back(card.id);
        }
    }

    if (eligibleCardIDs.empty()) {
        Log::Warning("No eligible cards to award");
        return nullptr;
    }

    // Pick random card
    i32 randomIndex = rand() % eligibleCardIDs.size();
    i32 cardID = eligibleCardIDs[randomIndex];

    return AddCardToCollection(cardID);
}

CardInstance* CardManager::AwardSpecificCard(i32 cardID) {
    return AddCardToCollection(cardID);
}

void CardManager::AwardStarterDeck() {
    // This will be populated from data/cards.json in Task 19
    // For now, just log that it was called
    Log::Info("Awarding starter deck (to be implemented with cards.json)");
}

// === Card Upgrades ===

bool CardManager::UpgradeCard(i32 instanceID, f64 qubitCost, f64 coherenceCost) {
    CardInstance* instance = GetCardInstance(instanceID);
    if (!instance) {
        Log::Warningf("Card instance not found: ", instanceID);
        return false;
    }

    Card* card = GetCardDefinition(instance->cardID);
    if (!card || !card->canUpgrade) {
        Log::Warning("Card cannot be upgraded");
        return false;
    }

    if (instance->upgradeLevel >= 3) {
        Log::Warning("Card is already max level");
        return false;
    }

    // TODO: Check if player can afford upgrade cost (qubitCost, coherenceCost)
    // For now, just assume they can

    instance->upgradeLevel++;
    Log::Infof("Upgraded card to level ", instance->upgradeLevel);

    return true;
}

i32 CardManager::GetUpgradeCost(i32 instanceID) const {
    const CardInstance* instance = GetCardInstance(instanceID);
    if (!instance) return 0;

    // Cost scales with upgrade level: 100, 200, 300
    return (instance->upgradeLevel + 1) * 100;
}

// === Save/Load ===

void CardManager::SaveToJson(std::ofstream& file) const {
    file << "    \"cardManager\": {\n";
    file << "        \"nextCardID\": " << m_NextCardID << ",\n";
    file << "        \"nextInstanceID\": " << m_NextInstanceID << ",\n";
    file << "        \"maxDeckSize\": " << m_MaxDeckSize << ",\n";
    file << "        \"maxHandSize\": " << m_MaxHandSize << ",\n";
    file << "        \"maxEnergy\": " << m_MaxEnergy << ",\n";
    file << "        \"totalCardsPlayed\": " << m_TotalCardsPlayed << ",\n";
    file << "        \"totalCardsDrawn\": " << m_TotalCardsDrawn << ",\n";

    // Save collection
    file << "        \"collection\": [\n";
    for (size_t i = 0; i < m_Collection.size(); i++) {
        const CardInstance* instance = m_Collection[i];
        file << "            {\"cardID\": " << instance->cardID
             << ", \"instanceID\": " << instance->instanceID
             << ", \"upgradeLevel\": " << instance->upgradeLevel
             << ", \"inDeck\": " << (instance->inDeck ? "true" : "false") << "}";
        if (i < m_Collection.size() - 1) file << ",";
        file << "\n";
    }
    file << "        ]\n";
    file << "    }";
}

void CardManager::LoadFromJson(const std::string& line) {
    // Simple manual JSON parsing (to be replaced with nlohmann/json later)
    // For now, just stub this out
    (void)line;
}

// === Private Helpers ===

CardInstance* CardManager::GetCardInstance(i32 instanceID) {
    for (CardInstance* instance : m_Collection) {
        if (instance->instanceID == instanceID) {
            return instance;
        }
    }
    return nullptr;
}

const CardInstance* CardManager::GetCardInstance(i32 instanceID) const {
    for (const CardInstance* instance : m_Collection) {
        if (instance->instanceID == instanceID) {
            return instance;
        }
    }
    return nullptr;
}

void CardManager::ApplyCardEffect(const CardEffect& effect, GameState* gameState, Enemy* target) {
    // This will delegate to GameState to apply effects
    // For now, just log what would happen
    (void)gameState;
    (void)target;

    switch (effect.type) {
        case CardType::Attack:
            Log::Infof("Card effect: Deal ", effect.value, " damage");
            break;
        case CardType::Defense:
            Log::Infof("Card effect: Gain ", effect.value, " defense");
            break;
        case CardType::ResourceGen:
            Log::Infof("Card effect: Generate ", effect.value, " resources");
            break;
        case CardType::Buff:
            Log::Infof("Card effect: Buff for ", effect.duration, " seconds");
            break;
        default:
            Log::Info("Card effect: Unknown effect type");
            break;
    }
}
