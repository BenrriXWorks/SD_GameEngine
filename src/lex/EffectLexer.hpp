#pragma once
#include <string>
#include <string_view>
#include <expected>
#include "../effects/impl/BasicEffects.hpp"
#include "../game/GameMap.hpp"

namespace lex {

// Error types for parsing
enum class ParseError : uint8_t {
    INVALID_INPUT,
    UNKNOWN_PREFIX,
    INCOMPLETE_INPUT,
    MALFORMED_STRING
};

// Effect type enumeration
enum class EffectType : uint8_t {
    UNKNOWN = 0,
    ATTACK_BUFF,
    ATTACK_DEBUFF,
    HEALTH_BUFF,
    HEALTH_DEBUFF,
    HEAL,
    RANGE_BUFF,
    RANGE_DEBUFF,
    DAMAGE,
    POSITIONAL_DEBUFF,
    POSITIONAL_TRIGGER,
    POSITION_ENTER_EFFECT,
    ADJACENCY_BUFF,
    RANGE_EFFECT,
    // Nuevos efectos de selección
    TARGETED_HEAL,
    TARGETED_DAMAGE,
    TARGETED_BUFF,
    TARGETED_DEBUFF
};

class EffectLexer {
private:
    // Fast character-by-character lexer using goto states
    static std::expected<EffectType, ParseError> parseEffectTypeImpl(const char* input);
    static std::expected<AttackModifierEffect::TargetType, ParseError> parseTargetTypeImpl(const char* input);
    static std::expected<GameMap::Adjacency, ParseError> parseDirectionImpl(const char* input);

public:
    // Main parsing functions - work directly with C strings
    static EffectType parseEffectType(const char* input) {
        if (!input || !*input) return EffectType::UNKNOWN;
        auto result = parseEffectTypeImpl(input);
        return result.value_or(EffectType::UNKNOWN);
    }
    
    static AttackModifierEffect::TargetType parseTargetType(const char* input) {
        if (!input || !*input) return AttackModifierEffect::TargetType::SELF;
        auto result = parseTargetTypeImpl(input);
        return result.value_or(AttackModifierEffect::TargetType::SELF);
    }
    
    static GameMap::Adjacency parseDirection(const char* input) {
        if (!input || !*input) return GameMap::Adjacency::UP; // Default fallback
        auto result = parseDirectionImpl(input);
        return result.value_or(GameMap::Adjacency::UP);
    }
    
    // Convenience overloads for std::string compatibility
    static EffectType parseEffectType(const std::string& input) {
        return parseEffectType(input.c_str());
    }
    
    static AttackModifierEffect::TargetType parseTargetType(const std::string& input) {
        return parseTargetType(input.c_str());
    }
    
    static GameMap::Adjacency parseDirection(const std::string& input) {
        return parseDirection(input.c_str());
    }
    
    // Overloads for std::string_view compatibility
    static EffectType parseEffectType(std::string_view input) {
        // Create null-terminated string for C-style parsing
        std::string temp(input);
        return parseEffectType(temp.c_str());
    }
    
    static AttackModifierEffect::TargetType parseTargetType(std::string_view input) {
        std::string temp(input);
        return parseTargetType(temp.c_str());
    }
    
    static GameMap::Adjacency parseDirection(std::string_view input) {
        std::string temp(input);
        return parseDirection(temp.c_str());
    }
};

} // namespace lex
