#include "EffectDispatch.hpp"

// Precomputed effect creator table (compile-time initialized)
const std::array<EffectDispatch::EffectCreator, static_cast<size_t>(lex::EffectType::TARGETED_DEBUFF) + 1> 
EffectDispatch::EFFECT_CREATORS = {{
    // UNKNOWN
    nullptr,
    
    // ATTACK_BUFF
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        return EffectFactory::createAttackModifier(source, owner, config.value, 
            lex::EffectLexer::parseTargetType(config.target));
    },
    
    // ATTACK_DEBUFF
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        return EffectFactory::createAttackModifier(source, owner, -config.value, 
            lex::EffectLexer::parseTargetType(config.target));
    },
    
    // HEALTH_BUFF
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        return EffectFactory::createHealthModifier(source, owner, config.value, 
            lex::EffectLexer::parseTargetType(config.target));
    },
    
    // HEALTH_DEBUFF
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        return EffectFactory::createHealthModifier(source, owner, -config.value, 
            lex::EffectLexer::parseTargetType(config.target));
    },
    
    // HEAL
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        return EffectFactory::createHealthModifier(source, owner, config.value, 
            lex::EffectLexer::parseTargetType(config.target));
    },
    
    // RANGE_BUFF
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        return EffectFactory::createRangeModifier(source, owner, config.value, 
            lex::EffectLexer::parseTargetType(config.target));
    },
    
    // RANGE_DEBUFF
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        return EffectFactory::createRangeModifier(source, owner, -config.value, 
            lex::EffectLexer::parseTargetType(config.target));
    },
    
    // DAMAGE
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        if (config.target == "specific_position") {
            return EffectFactory::createTargetedSpell(source, owner, "Damage Spell", config.x, config.y, config.value);
        }
        return EffectPtr(nullptr);
    },
    
    // POSITIONAL_DEBUFF
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        GameMap::Adjacency direction = lex::EffectLexer::parseDirection(config.direction);
        return EffectFactory::createPositionalDebuff(source, owner, "Positional Debuff", direction, config.value);
    },
    
    // POSITIONAL_TRIGGER
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        GameMap::Adjacency direction = lex::EffectLexer::parseDirection(config.direction);
        auto triggeredEffect = EffectFactory::createAttackModifier(source, owner, 1, AttackModifierEffect::TargetType::SELF);
        return EffectFactory::createPositionalTrigger(source, owner, direction, triggeredEffect);
    },
    
    // POSITION_ENTER_EFFECT
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        GameMap::Adjacency direction = lex::EffectLexer::parseDirection(config.direction);
        return EffectFactory::createOnPositionEnterEffect(source, owner, "On Position Enter", direction);
    },
    
    // ADJACENCY_BUFF
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        int half = config.value / 2;
        return EffectFactory::createAdjacentBuff(source, owner, "Adjacency Buff", half, config.value - half);
    },
    
    // RANGE_EFFECT
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        return EffectFactory::createRangeBasedEffect(source, owner, "Range Effect", 2, config.value);
    },
    
    // TARGETED_HEAL
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        auto effect = EffectFactory::createTargetedHeal(source, owner, config.value);
        // Si se especifica posición objetivo en el config, establecerla
        if (config.x != 0 || config.y != 0) {
            auto selectionEffect = std::dynamic_pointer_cast<TargetedHealEffect>(effect);
            if (selectionEffect) {
                selectionEffect->setTarget(config.x, config.y);
            }
        }
        return effect;
    },
    
    // TARGETED_DAMAGE
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        auto effect = EffectFactory::createTargetedDamage(source, owner, config.value);
        // Si se especifica posición objetivo en el config, establecerla
        if (config.x != 0 || config.y != 0) {
            auto selectionEffect = std::dynamic_pointer_cast<TargetedDamageEffect>(effect);
            if (selectionEffect) {
                selectionEffect->setTarget(config.x, config.y);
            }
        }
        return effect;
    },
    
    // TARGETED_BUFF
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        // Usar value como attack buff, y 1 como health buff por defecto
        auto effect = EffectFactory::createTargetedBuff(source, owner, config.value, 1);
        // Si se especifica posición objetivo en el config, establecerla
        if (config.x != 0 || config.y != 0) {
            auto selectionEffect = std::dynamic_pointer_cast<TargetedBuffEffect>(effect);
            if (selectionEffect) {
                selectionEffect->setTarget(config.x, config.y);
            }
        }
        return effect;
    },
    
    // TARGETED_DEBUFF
    [](const CardLoader::EffectConfig& config, CardPtr source, PlayerId owner) {
        // Usar value como attack debuff, y -1 como speed debuff por defecto
        auto effect = EffectFactory::createTargetedDebuff(source, owner, -config.value, -1);
        // Si se especifica posición objetivo en el config, establecerla
        if (config.x != 0 || config.y != 0) {
            auto selectionEffect = std::dynamic_pointer_cast<TargetedDebuffEffect>(effect);
            if (selectionEffect) {
                selectionEffect->setTarget(config.x, config.y);
            }
        }
        return effect;
    }
}};

EffectDispatch::EffectCreator EffectDispatch::lookup(std::string_view effectType) {
    lex::EffectType type = lex::EffectLexer::parseEffectType(effectType);
    if (type != lex::EffectType::UNKNOWN) {
        return EFFECT_CREATORS[static_cast<size_t>(type)];
    }
    return nullptr;
}
