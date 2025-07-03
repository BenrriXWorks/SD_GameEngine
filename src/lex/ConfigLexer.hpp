#pragma once
#include "../game/GameMap.hpp"
#include "../utils/Types.hpp"
#include <string>
#include <vector>

// Lexer para convertir strings de configuración JSON a enums
class ConfigLexer {
public:
    // Convertir string de dirección a GameMap::Adjacency
    static GameMap::Adjacency parseDirection(const std::string& direction) {
            if (direction == "up") return GameMap::Adjacency::UP;
            if (direction == "down") return GameMap::Adjacency::DOWN;
            if (direction == "top_left" || direction == "up_left") return GameMap::Adjacency::TOP_LEFT;
            if (direction == "top_right" || direction == "up_right") return GameMap::Adjacency::TOP_RIGHT;
            if (direction == "bottom_left" || direction == "down_left") return GameMap::Adjacency::BOTTOM_LEFT;
            if (direction == "bottom_right" || direction == "down_right") return GameMap::Adjacency::BOTTOM_RIGHT;
            
            // Valor por defecto
            return GameMap::Adjacency::UP;
        }
    
    // Convertir array de strings de direcciones a vector de GameMap::Adjacency
    static std::vector<GameMap::Adjacency> parseDirections(const std::vector<std::string>& directions) {
        std::vector<GameMap::Adjacency> result;
        for (const auto& dir : directions) {
            result.push_back(parseDirection(dir));
        }
        return result;
    }
    
    // Enum para tipos de target
    enum class TargetType {
        SELF,
        ADJACENT,
        ALL_ALLIES,
        ALL_ENEMIES,
        ATTACK_TARGET,
        ATTACKER,
        SPECIFIC_POSITION,
        GAME_STATE
    };
    
    // Convertir string de target type a enum
    static TargetType parseTargetType(const std::string& targetType) {
        if (targetType == "self") return TargetType::SELF;
        if (targetType == "adjacent") return TargetType::ADJACENT;
        if (targetType == "all_allies") return TargetType::ALL_ALLIES;
        if (targetType == "all_enemies") return TargetType::ALL_ENEMIES;
        if (targetType == "attack_target") return TargetType::ATTACK_TARGET;
        if (targetType == "attacker") return TargetType::ATTACKER;
        if (targetType == "specific_position") return TargetType::SPECIFIC_POSITION;
        if (targetType == "game_state") return TargetType::GAME_STATE;
        
        // Valor por defecto
        return TargetType::SELF;
    }
    
    // Enum para tipos de trigger
    enum class TriggerType {
        ON_PLAY,
        ON_ATTACK,
        ON_ATTACKED,
        ON_KILL,
        ON_DEATH,
        TURN_START,
        TURN_END
    };
    
    // Convertir string de trigger type a enum
    static TriggerType parseTriggerType(const std::string& triggerType) {
        if (triggerType == "on_play") return TriggerType::ON_PLAY;
        if (triggerType == "on_attack") return TriggerType::ON_ATTACK;
        if (triggerType == "on_attacked") return TriggerType::ON_ATTACKED;
        if (triggerType == "on_kill") return TriggerType::ON_KILL;
        if (triggerType == "on_death") return TriggerType::ON_DEATH;
        if (triggerType == "turn_start") return TriggerType::TURN_START;
        if (triggerType == "turn_end") return TriggerType::TURN_END;
        
        // Valor por defecto
        return TriggerType::ON_PLAY;
    }
    
    // Enum para tipos de efecto
    enum class EffectType {
        ATTRIBUTE_MODIFIER,
        DIRECT_DAMAGE,
        HEAL,
        REFLECT_DAMAGE,
        DRAW_CARDS,
        DESTROY
    };
    
    // Convertir string de effect type a enum
    static EffectType parseEffectType(const std::string& effectType) {
        if (effectType == "attribute_modifier") return EffectType::ATTRIBUTE_MODIFIER;
        if (effectType == "direct_damage") return EffectType::DIRECT_DAMAGE;
        if (effectType == "heal") return EffectType::HEAL;
        if (effectType == "reflect_damage") return EffectType::REFLECT_DAMAGE;
        if (effectType == "draw_cards") return EffectType::DRAW_CARDS;
        if (effectType == "destroy") return EffectType::DESTROY;
        
        // Valor por defecto
        return EffectType::ATTRIBUTE_MODIFIER;
    }
    
    // Enum para filtros de target
    enum class TargetFilter {
        ANY,
        ALLIES_ONLY,
        ENEMIES_ONLY
    };
    
    // Convertir string de filter a enum
    static TargetFilter parseTargetFilter(const std::string& filter) {
        if (filter == "allies_only") return TargetFilter::ALLIES_ONLY;
        if (filter == "enemies_only") return TargetFilter::ENEMIES_ONLY;
        if (filter == "any") return TargetFilter::ANY;
        
        // Valor por defecto
        return TargetFilter::ANY;
    }
};
