#pragma once
#include "Effect.hpp"
#include "../game/GameMap.hpp"
#include "impl/BasicEffects.hpp"
#include "impl/PositionalDebuffEffect.hpp"
#include "impl/TargetedSpellEffect.hpp"
#include "impl/OnPositionEnterEffect.hpp"
#include "impl/EfficientEffects.hpp"
#include "impl/SelectionEffects.hpp"
#include <memory>

class EffectFactory {
public:
    // Factory methods to create different types of effects
    
    // Attack modifier effect (buff/debuff)
    static EffectPtr createAttackModifier(CardPtr source, PlayerId owner, 
                                        int attackChange, 
                                        AttackModifierEffect::TargetType targetType) {
        return std::make_shared<AttackModifierEffect>(source, owner, attackChange, targetType);
    }
    
    // Health modifier effect (heal/damage)
    static EffectPtr createHealthModifier(CardPtr source, PlayerId owner, 
                                        int healthChange, 
                                        AttackModifierEffect::TargetType targetType) {
        return std::make_shared<HealthModifierEffect>(source, owner, healthChange, targetType);
    }
    
    // Range modifier effect
    static EffectPtr createRangeModifier(CardPtr source, PlayerId owner, 
                                        int rangeChange, 
                                        AttackModifierEffect::TargetType targetType) {
        return std::make_shared<RangeModifierEffect>(source, owner, rangeChange, targetType);
    }
    
    // Positional trigger effect
    static EffectPtr createPositionalTrigger(CardPtr source, PlayerId owner, 
                                           GameMap::Adjacency direction, 
                                           EffectPtr triggeredEffect) {
        return std::make_shared<PositionalTriggerEffect>(source, owner, direction, triggeredEffect);
    }
    
    // Targeted spell effect
    static EffectPtr createTargetedSpell(CardPtr source, PlayerId owner, 
                                        const std::string& name,
                                        uint8_t x, uint8_t y, 
                                        uint8_t damage, 
                                        bool isInstant = true) {
        return std::make_shared<TargetedSpellEffect>(source, owner, name, x, y, damage, isInstant);
    }
    
    // Positional debuff effect
    static EffectPtr createPositionalDebuff(CardPtr source, PlayerId owner, 
                                          const std::string& name,
                                          GameMap::Adjacency direction, 
                                          uint8_t attackReduction = 1) {
        return std::make_shared<PositionalDebuffEffect>(source, owner, name, direction, attackReduction);
    }
    
    // On position enter effect
    static EffectPtr createOnPositionEnterEffect(CardPtr source, PlayerId owner, 
                                              const std::string& name,
                                              GameMap::Adjacency direction) {
        return std::make_shared<OnPositionEnterEffect>(source, owner, name, direction);
    }
    
    // Adjacent buff effect (efficient implementation)
    static EffectPtr createAdjacentBuff(CardPtr source, PlayerId owner, 
                                      const std::string& name,
                                      uint8_t attackBonus, 
                                      uint8_t healthBonus) {
        return std::make_shared<AdjacentBuffEffect>(source, owner, name, attackBonus, healthBonus);
    }
    
    // Range-based effect (efficient implementation)
    static EffectPtr createRangeBasedEffect(CardPtr source, PlayerId owner, 
                                          const std::string& name,
                                          uint8_t range, 
                                          int8_t attackModifier) {
        return std::make_shared<RangeBasedEffect>(source, owner, name, range, attackModifier);
    }
    
    // Nuevos métodos para efectos de selección
    
    // Targeted heal effect (seleccionar aliado para curar)
    static EffectPtr createTargetedHeal(CardPtr source, PlayerId owner, uint8_t healAmount) {
        return std::make_shared<TargetedHealEffect>(source, owner, healAmount);
    }
    
    // Targeted damage effect (seleccionar enemigo para dañar, no leyenda)
    static EffectPtr createTargetedDamage(CardPtr source, PlayerId owner, uint8_t damage) {
        return std::make_shared<TargetedDamageEffect>(source, owner, damage);
    }
    
    // Targeted buff effect (seleccionar aliado para buffear)
    static EffectPtr createTargetedBuff(CardPtr source, PlayerId owner, int attackBuff, int healthBuff) {
        return std::make_shared<TargetedBuffEffect>(source, owner, attackBuff, healthBuff);
    }
    
    // Targeted debuff effect (seleccionar enemigo para debuffear, no leyenda)
    static EffectPtr createTargetedDebuff(CardPtr source, PlayerId owner, int attackDebuff, int speedDebuff) {
        return std::make_shared<TargetedDebuffEffect>(source, owner, attackDebuff, speedDebuff);
    }
};
