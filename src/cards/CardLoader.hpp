#pragma once
#include "../cards/Card.hpp"
#include "../effects/EffectFactory.hpp"
#include "../effects/impl/BasicEffects.hpp"
#include "../game/GameMap.hpp"
#include <vector>
#include <string>
#include <memory>
#include "../../libs/json.hpp" // nlohmann/json

class CardLoader {
public:
    struct EffectConfig {
        std::string type;           // "attack_buff", "health_debuff", "damage", etc.
        std::string target;         // "self", "adjacent", "all_friendly", "all_enemy", "specific_position"
        int value = 0;              // The effect value (positive or negative)
        std::string trigger;        // "on_play", "on_enter_position", "on_start_turn", etc.
        std::string direction = ""; // For positional effects: "up", "down", "left", "right", etc.
        uint8_t x = 0, y = 0;      // For specific position targets
    };
    
    struct CardConfig {
        uint32_t id;
        std::string name;
        std::string description;
        uint8_t cost;
        std::string type;           // "unit", "legend", or "spell"
        
        // Unit-specific
        uint8_t attack = 0;
        uint8_t health = 0;
        uint8_t speed = 1;
        uint8_t range = 1;
        
        std::vector<EffectConfig> effects;
    };
    
    struct DeckConfig {
        std::string name;
        std::vector<CardConfig> cards;
    };
    
    // Load deck configuration from JSON file
    static std::vector<DeckConfig> loadDecksFromFile(const std::string& filename);
    
    // Create actual card objects from configuration
    static std::vector<CardPtr> createCardsFromConfig(const DeckConfig& deckConfig, PlayerId owner);
    
private:
    // Helper methods for parsing
    static EffectConfig parseEffect(const nlohmann::json& effectJson);
    static CardConfig parseCard(const nlohmann::json& cardJson);
    static DeckConfig parseDeck(const nlohmann::json& deckJson);
    
    // Helper to convert string to TargetType
    static AttackModifierEffect::TargetType parseTargetType(const std::string& target);
    
    // Helper to convert string to Adjacency
    static GameMap::Adjacency parseDirection(const std::string& direction);
    
    // Helper to create effect from config
    static EffectPtr createEffectFromConfig(const EffectConfig& config, CardPtr source, PlayerId owner);
};
