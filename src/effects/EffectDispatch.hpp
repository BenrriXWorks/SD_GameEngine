#pragma once
#include <array>
#include <string_view>
#include <functional>
#include "Effect.hpp"
#include "EffectFactory.hpp"
#include "../cards/CardLoader.hpp"
#include "../lex/EffectLexer.hpp"

class EffectDispatch {
public:
    // Function signature for effect creators
    using EffectCreator = std::function<EffectPtr(const CardLoader::EffectConfig&, CardPtr, PlayerId)>;
    
    // Effect creator function table (indexed by lex::EffectType)
    static const std::array<EffectCreator, static_cast<size_t>(lex::EffectType::TARGETED_DEBUFF) + 1> EFFECT_CREATORS;

public:
    // Main lookup function - O(1) with lexer-based parsing
    static EffectCreator lookup(std::string_view effectType);
    
    // Fast parsing utilities using the lexer
    static AttackModifierEffect::TargetType getTargetType(std::string_view target) {
        return lex::EffectLexer::parseTargetType(target);
    }
    
    static GameMap::Adjacency getDirection(std::string_view direction) {
        return lex::EffectLexer::parseDirection(direction);
    }
};
