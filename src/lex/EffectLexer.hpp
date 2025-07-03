#pragma once
#include <string>
#include <string_view>
#include <expected>
#include "../effects/impl/EffectImplLibrary.hpp"
#include "../game/GameMap.hpp"
#include "ConfigLexer.hpp"

namespace lex {

// Error types for parsing
enum class ParseError : uint8_t {
    INVALID_INPUT,
    UNKNOWN_PREFIX,
    INCOMPLETE_INPUT,
    MALFORMED_STRING
};

class EffectLexer {
private:
    // Fast character-by-character lexer using goto states
    static std::expected<ConfigLexer::EffectType, ParseError> parseEffectTypeImpl(const char* input);
    static std::expected<ConfigLexer::TargetType, ParseError> parseTargetTypeImpl(const char* input);
    static std::expected<GameMap::Adjacency, ParseError> parseDirectionImpl(const char* input);
    static std::expected<ConfigLexer::TriggerType, ParseError> parseTriggerTypeImpl(const char* input);

public:
    // Main parsing functions - work directly with C strings
    static ConfigLexer::EffectType parseEffectType(const char* input) {
        if (!input || !*input) return ConfigLexer::EffectType::ATTRIBUTE_MODIFIER;
        auto result = parseEffectTypeImpl(input);
        return result.value_or(ConfigLexer::EffectType::ATTRIBUTE_MODIFIER);
    }
    
    static ConfigLexer::TargetType parseTargetType(const char* input) {
        if (!input || !*input) return ConfigLexer::TargetType::SELF;
        auto result = parseTargetTypeImpl(input);
        return result.value_or(ConfigLexer::TargetType::SELF);
    }
    
    static GameMap::Adjacency parseDirection(const char* input) {
        if (!input || !*input) return GameMap::Adjacency::UP; // Default fallback
        auto result = parseDirectionImpl(input);
        return result.value_or(GameMap::Adjacency::UP);
    }
    
    static ConfigLexer::TriggerType parseTriggerType(const char* input) {
        if (!input || !*input) return ConfigLexer::TriggerType::ON_PLAY;
        auto result = parseTriggerTypeImpl(input);
        return result.value_or(ConfigLexer::TriggerType::ON_PLAY);
    }
    
    // Convenience overloads for std::string compatibility
    static ConfigLexer::EffectType parseEffectType(const std::string& input) {
        return parseEffectType(input.c_str());
    }
    
    static ConfigLexer::TargetType parseTargetType(const std::string& input) {
        return parseTargetType(input.c_str());
    }
    
    static GameMap::Adjacency parseDirection(const std::string& input) {
        return parseDirection(input.c_str());
    }
    
    static ConfigLexer::TriggerType parseTriggerType(const std::string& input) {
        return parseTriggerType(input.c_str());
    }
    
    // Overloads for std::string_view compatibility
    static ConfigLexer::EffectType parseEffectType(std::string_view input) {
        // Create null-terminated string for C-style parsing
        std::string temp(input);
        return parseEffectType(temp.c_str());
    }
    
    static ConfigLexer::TargetType parseTargetType(std::string_view input) {
        std::string temp(input);
        return parseTargetType(temp.c_str());
    }
    
    static GameMap::Adjacency parseDirection(std::string_view input) {
        std::string temp(input);
        return parseDirection(temp.c_str());
    }
    
    static ConfigLexer::TriggerType parseTriggerType(std::string_view input) {
        std::string temp(input);
        return parseTriggerType(temp.c_str());
    }
};

} // namespace lex
