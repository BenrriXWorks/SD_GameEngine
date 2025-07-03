#pragma once
#include "../cards/Card.hpp"
#include "../effects/ComposedEffectFactory.hpp"
#include "../game/GameMap.hpp"
#include "../lex/ConfigLexer.hpp"
#include <vector>
#include <string>
#include <memory>
#include "../../libs/json.hpp" // nlohmann/json

class CardLoader {
public:
    struct EffectConfig {
        ConfigLexer::EffectType type;           // "attribute_modifier", "direct_damage", "heal", etc.
        ConfigLexer::TargetType target_type;    // "self", "adjacent", "all_allies", "attack_target", etc.
        ConfigLexer::TargetFilter filter;       // "allies_only", "enemies_only", "any"
        int value = 0;                          // The effect value (positive or negative)
        ConfigLexer::TriggerType trigger;       // "on_play", "on_attack", "on_death", "turn_start", etc.
        std::string attribute = "";             // For attribute modifiers: "attack", "health", "speed"
        int duration = 0;                       // Duration: 0=instant, 1+=turns, -1=persistent
        std::vector<GameMap::Adjacency> directions; // For adjacent targeting: parsed directions
        uint8_t x = 0, y = 0;                  // For specific position targets
        
        // Helper method to determine if this effect uses adjacency
        bool isAdjacencyEffect() const {
            return target_type == ConfigLexer::TargetType::ADJACENT && !directions.empty();
        }
        
        // Helper method to determine if effect is persistent
        bool isPersistent() const {
            return duration == -1;
        }
        
        // Helper method to determine if effect is instant
        bool isInstant() const {
            return duration == 0;
        }
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
    
    // Helper to convert string to Adjacency
    static GameMap::Adjacency parseDirection(const std::string& direction);
    
    // Helper to create effect from config
    static EffectPtr createEffectFromConfig(const EffectConfig& config, CardPtr source, PlayerId owner);
    
    // Método para crear un efecto compuesto a partir de la configuración
    static EffectPtr createComposedEffectFromConfig(const EffectConfig& config, CardPtr source, PlayerId owner) {
        // Usar el nuevo método con enums
        return ComposedEffectFactory::createFromConfig(
            config.type, config.trigger, config.target_type, source, owner, 
            config.value, config.filter, config.directions, config.attribute
        );
    }
};
