#pragma once

#include "Card.h"
#include "Types.h"
#include <string>
#include <vector>
#include <unordered_map>

// Forward declarations
class GameState;
class Enemy;
class Renderer;

// Card Manager - Handles quantum deck building and card gameplay
class CardManager {
public:
    CardManager();

    void Initialize();
    void Update(f64 deltaTime);

    // === Card Database ===
    void RegisterCard(const Card& card);
    Card* GetCardDefinition(i32 cardID);
    const Card* GetCardDefinition(i32 cardID) const;
    std::vector<const Card*> GetAllCards() const;
    std::vector<const Card*> GetCardsByRarity(CardRarity rarity) const;

    // === Player Collection ===
    void UnlockCard(i32 cardID);
    bool IsCardUnlocked(i32 cardID) const;
    CardInstance* AddCardToCollection(i32 cardID);
    std::vector<CardInstance*> GetCollection();
    const std::vector<CardInstance*>& GetCollection() const;
    i32 GetCollectionSize() const;

    // === Deck Building ===
    bool AddCardToDeck(i32 instanceID);
    bool RemoveCardFromDeck(i32 instanceID);
    void ClearDeck();
    std::vector<CardInstance*> GetDeck();
    i32 GetDeckSize() const;
    i32 GetMaxDeckSize() const { return m_MaxDeckSize; }
    void SetMaxDeckSize(i32 size) { m_MaxDeckSize = size; }

    // === Hand Management ===
    void StartCombat(); // Called when combat begins - shuffles deck and draws hand
    void EndCombat();   // Called when combat ends - clears hand
    void DrawCard();
    void DrawCards(i32 count);
    void DiscardCard(i32 handIndex);
    void DiscardHand();
    void ShuffleDeck();

    std::vector<CardInstance*> GetHand();
    const std::vector<CardInstance*>& GetHand() const;
    i32 GetHandSize() const;
    i32 GetMaxHandSize() const { return m_MaxHandSize; }
    void SetMaxHandSize(i32 size) { m_MaxHandSize = size; }

    // === Playing Cards ===
    bool CanPlayCard(i32 handIndex, f64 qubits, f64 coherence, i32 energy) const;
    bool PlayCard(i32 handIndex, GameState* gameState, Enemy* target = nullptr);

    // === Card Acquisition ===
    CardInstance* AwardRandomCard(CardRarity minRarity = CardRarity::Common);
    CardInstance* AwardSpecificCard(i32 cardID);
    void AwardStarterDeck(); // Give player starting cards

    // === Card Upgrades ===
    bool UpgradeCard(i32 instanceID, f64 qubitCost, f64 coherenceCost);
    i32 GetUpgradeCost(i32 instanceID) const;

    // === Energy System (for combat) ===
    void SetEnergy(i32 energy) { m_CurrentEnergy = energy; }
    void AddEnergy(i32 amount) { m_CurrentEnergy += amount; }
    i32 GetEnergy() const { return m_CurrentEnergy; }
    i32 GetMaxEnergy() const { return m_MaxEnergy; }
    void SetMaxEnergy(i32 energy) { m_MaxEnergy = energy; }

    // === Statistics ===
    i32 GetCardsPlayed() const { return m_TotalCardsPlayed; }
    i32 GetCardsDrawn() const { return m_TotalCardsDrawn; }

    // === Save/Load ===
    void SaveToJson(std::ofstream& file) const;
    void LoadFromJson(const std::string& line);

private:
    // Card database (all possible cards)
    std::unordered_map<i32, Card> m_CardDatabase;
    i32 m_NextCardID;

    // Player collection (owned card instances)
    std::vector<CardInstance*> m_Collection;
    i32 m_NextInstanceID;

    // Active deck (subset of collection used in combat)
    std::vector<i32> m_DeckInstanceIDs; // References to collection
    i32 m_MaxDeckSize;

    // Hand (cards drawn from deck during combat)
    std::vector<i32> m_HandInstanceIDs; // References to collection
    i32 m_MaxHandSize;

    // Draw pile (cards not yet drawn in current combat)
    std::vector<i32> m_DrawPileInstanceIDs;

    // Discard pile (cards played/discarded in current combat)
    std::vector<i32> m_DiscardPileInstanceIDs;

    // Combat energy system
    i32 m_CurrentEnergy;
    i32 m_MaxEnergy;

    // Statistics
    i32 m_TotalCardsPlayed;
    i32 m_TotalCardsDrawn;

    // Helper methods
    CardInstance* GetCardInstance(i32 instanceID);
    const CardInstance* GetCardInstance(i32 instanceID) const;
    void ApplyCardEffect(const CardEffect& effect, GameState* gameState, Enemy* target);
};
