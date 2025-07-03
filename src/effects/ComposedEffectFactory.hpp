#pragma once
#include "ComposedEffect.hpp"
#include "impl/EffectImplLibrary.hpp"
#include "../triggers/TriggerFactory.hpp"
#include "../targeting/TargetSelectorFactory.hpp"
#include "../game/GameMap.hpp"
#include "../lex/ConfigLexer.hpp"
#include <memory>
#include <vector>
#include <string>

class ComposedEffectFactory {
public:
    // Método para crear un efecto de modificación de ataque con targeting
    static EffectPtr createAttackEffect(CardPtr source, PlayerId owner, 
                                      int attackChange, 
                                      TargetSelectorPtr targetSelector,
                                      TriggerPtr trigger) {
        auto impl = std::static_pointer_cast<EffectImpl>(std::make_shared<AttackModifierEffectImpl>(source, owner, attackChange));
        return std::make_shared<ComposedEffect>(impl, trigger, targetSelector, source, owner);
    }
    
    // Método para crear un efecto de modificación de ataque con múltiples triggers
    static EffectPtr createAttackEffect(CardPtr source, PlayerId owner, 
                                      int attackChange, 
                                      TargetSelectorPtr targetSelector,
                                      const std::vector<TriggerPtr>& triggers) {
        auto impl = std::static_pointer_cast<EffectImpl>(std::make_shared<AttackModifierEffectImpl>(source, owner, attackChange));
        return std::make_shared<ComposedEffect>(impl, triggers, targetSelector, source, owner);
    }
    
    // Método para crear un efecto de daño
    static EffectPtr createDamageEffect(CardPtr source, PlayerId owner, 
                                      uint8_t damage,
                                      TargetSelectorPtr targetSelector,
                                      TriggerPtr trigger) {
        auto impl = std::make_shared<DamageEffectImpl>(source, owner, damage);
        return std::make_shared<ComposedEffect>(impl, trigger, targetSelector, source, owner);
    }
    
    // Método para crear un efecto de curación
    static EffectPtr createHealEffect(CardPtr source, PlayerId owner, 
                                    uint8_t healAmount,
                                    TargetSelectorPtr targetSelector,
                                    TriggerPtr trigger) {
        auto impl = std::make_shared<HealEffectImpl>(source, owner, healAmount);
        return std::make_shared<ComposedEffect>(impl, trigger, targetSelector, source, owner);
    }
    
    // Método para crear un efecto de robar cartas
    static EffectPtr createDrawCardsEffect(CardPtr source, PlayerId owner, 
                                         uint8_t cardCount,
                                         TriggerPtr trigger) {
        auto impl = std::make_shared<DrawCardsEffectImpl>(source, owner, cardCount);
        auto gameStateSelector = TargetSelectorFactory::createGameStateSelector(source, owner);
        return std::make_shared<ComposedEffect>(impl, trigger, gameStateSelector, source, owner);
    }
    
    // Método principal para crear efectos desde configuración JSON usando enums
    static EffectPtr createFromConfig(ConfigLexer::EffectType effectType, 
                                    ConfigLexer::TriggerType triggerType,
                                    ConfigLexer::TargetType targetType,
                                    CardPtr source, PlayerId owner,
                                    int value,
                                    ConfigLexer::TargetFilter targetFilter = ConfigLexer::TargetFilter::ANY,
                                    const std::vector<GameMap::Adjacency>& directions = {},
                                    const std::string& attribute = "",
                                    uint8_t x = 0, uint8_t y = 0) {
        
        // Crear trigger usando enums
        TriggerPtr trigger = nullptr;
        switch (triggerType) {
            case ConfigLexer::TriggerType::ON_PLAY:
                trigger = TriggerFactory::createOnPlayTrigger(source, owner);
                break;
            case ConfigLexer::TriggerType::ON_ATTACK:
                trigger = TriggerFactory::createOnAttackTrigger(source, owner);
                break;
            case ConfigLexer::TriggerType::ON_ATTACKED:
                trigger = TriggerFactory::createOnAttackedTrigger(source, owner);
                break;
            case ConfigLexer::TriggerType::ON_KILL:
                trigger = TriggerFactory::createOnKillTrigger(source, owner);
                break;
            case ConfigLexer::TriggerType::ON_DEATH:
                trigger = TriggerFactory::createOnDeathTrigger(source, owner);
                break;
            case ConfigLexer::TriggerType::TURN_START:
                trigger = TriggerFactory::createOnStartTurnTrigger(source, owner);
                break;
            case ConfigLexer::TriggerType::TURN_END:
                trigger = TriggerFactory::createOnEndTurnTrigger(source, owner);
                break;
        }
        
        if (!trigger) return nullptr;
        
        // Crear target selector usando enums
        auto targetSelector = TargetSelectorFactory::createFromConfig(targetType, source, owner, 
                                                                     targetFilter, directions, x, y);
        if (!targetSelector) return nullptr;
        
        // Crear effect implementation según el tipo
        EffectImplPtr impl = nullptr;
        
        switch (effectType) {
            case ConfigLexer::EffectType::ATTRIBUTE_MODIFIER:
                // Crear el efecto apropiado según el atributo
                if (attribute == "attack") {
                    impl = std::make_shared<AttackModifierEffectImpl>(source, owner, value);
                } else if (attribute == "health") {
                    impl = std::make_shared<HealthModifierEffectImpl>(source, owner, value);
                } else if (attribute == "speed") {
                    impl = std::make_shared<SpeedModifierEffectImpl>(source, owner, value);
                } else if (attribute == "range") {
                    impl = std::make_shared<RangeModifierEffectImpl>(source, owner, value);
                } else {
                    // Por defecto, attack
                    impl = std::make_shared<AttackModifierEffectImpl>(source, owner, value);
                }
                break;
            case ConfigLexer::EffectType::DIRECT_DAMAGE:
                impl = std::make_shared<DamageEffectImpl>(source, owner, std::abs(value));
                break;
            case ConfigLexer::EffectType::HEAL:
                impl = std::make_shared<HealEffectImpl>(source, owner, std::abs(value));
                break;
            case ConfigLexer::EffectType::REFLECT_DAMAGE:
                impl = std::make_shared<DamageEffectImpl>(source, owner, std::abs(value)); // Simplificado por ahora
                break;
            case ConfigLexer::EffectType::DRAW_CARDS:
                impl = std::make_shared<DrawCardsEffectImpl>(source, owner, std::abs(value));
                targetSelector = TargetSelectorFactory::createGameStateSelector(source, owner);
                break;
            case ConfigLexer::EffectType::DESTROY:
                impl = std::make_shared<DestroyEffectImpl>(source, owner);
                break;
        }
        
        if (!impl) return nullptr;
        
        // Crear el efecto compuesto
        return std::make_shared<ComposedEffect>(impl, trigger, targetSelector, source, owner);
    }
    
    // Método principal para crear efectos desde configuración JSON (legacy - strings)
    static EffectPtr createFromConfig(const std::string& effectType, 
                                    const std::string& triggerType,
                                    const std::string& targetType,
                                    CardPtr source, PlayerId owner,
                                    int value,
                                    const std::string& targetFilter = "",
                                    const std::string& direction = "",
                                    uint8_t x = 0, uint8_t y = 0) {
        
        // Crear trigger - usar método simple para tipos básicos
        TriggerPtr trigger = nullptr;
        if (triggerType == "on_play") {
            trigger = TriggerFactory::createOnPlayTrigger(source, owner);
        } else if (triggerType == "on_attack") {
            trigger = TriggerFactory::createOnAttackTrigger(source, owner);
        } else if (triggerType == "on_attacked") {
            trigger = TriggerFactory::createOnAttackedTrigger(source, owner);
        } else if (triggerType == "on_kill") {
            trigger = TriggerFactory::createOnKillTrigger(source, owner);
        } else if (triggerType == "on_death") {
            trigger = TriggerFactory::createOnDeathTrigger(source, owner);
        } else if (triggerType == "turn_start") {
            trigger = TriggerFactory::createOnStartTurnTrigger(source, owner);
        } else if (triggerType == "turn_end") {
            trigger = TriggerFactory::createOnEndTurnTrigger(source, owner);
        } else {
            // Trigger tipo desconocido, usar OnPlay por defecto
            trigger = TriggerFactory::createOnPlayTrigger(source, owner);
        }
        if (!trigger) return nullptr;
        
        // Crear target selector
        auto targetSelector = TargetSelectorFactory::createFromConfig(targetType, source, owner, 
                                                                     targetFilter, direction, x, y);
        if (!targetSelector) return nullptr;
        
        // Crear effect implementation según el tipo
        EffectImplPtr impl = nullptr;
        
        if (effectType == "attack_modifier" || effectType == "attack_buff" || effectType == "attack_debuff") {
            impl = std::make_shared<AttackModifierEffectImpl>(source, owner, value);
        } else if (effectType == "health_modifier" || effectType == "health_buff" || effectType == "health_debuff") {
            impl = std::make_shared<HealthModifierEffectImpl>(source, owner, value);
        } else if (effectType == "damage") {
            impl = std::make_shared<DamageEffectImpl>(source, owner, std::abs(value));
        } else if (effectType == "heal") {
            impl = std::make_shared<HealEffectImpl>(source, owner, std::abs(value));
        } else if (effectType == "range_modifier" || effectType == "range_buff" || effectType == "range_debuff") {
            impl = std::make_shared<RangeModifierEffectImpl>(source, owner, value);
        } else if (effectType == "speed_modifier" || effectType == "speed_buff" || effectType == "speed_debuff") {
            impl = std::make_shared<SpeedModifierEffectImpl>(source, owner, value);
        } else if (effectType == "draw_cards") {
            impl = std::make_shared<DrawCardsEffectImpl>(source, owner, std::abs(value));
            // Para efectos de estado de juego, forzar GameStateTargetSelector
            targetSelector = TargetSelectorFactory::createGameStateSelector(source, owner);
        } else if (effectType == "add_mana") {
            impl = std::make_shared<AddManaEffectImpl>(source, owner, std::abs(value));
            targetSelector = TargetSelectorFactory::createGameStateSelector(source, owner);
        } else {
            // Tipo de efecto desconocido
            return nullptr;
        }
        
        if (!impl) return nullptr;
        
        // Crear el efecto compuesto
        return std::make_shared<ComposedEffect>(impl, trigger, targetSelector, source, owner);
    }
    
    // Método legacy para compatibilidad con código existente
    static EffectPtr createFromConfig(const std::string& effectType, 
                                    const std::string& triggerType,
                                    CardPtr source, PlayerId owner,
                                    int value,
                                    const std::string& target,
                                    const std::string& direction = "") {
        
        // Mapear target legacy a nuevo sistema
        std::string newTargetType = target;
        std::string targetFilter = "";
        
        if (target == "adjacent") {
            newTargetType = "adjacent";
            targetFilter = "all";
        } else if (target == "all_friendly") {
            newTargetType = "all_allies";
        } else if (target == "all_enemy") {
            newTargetType = "all_enemies";
        }
        
        return createFromConfig(effectType, triggerType, newTargetType, source, owner, 
                              value, targetFilter, direction, 0, 0);
    }
    
    // Método para crear un efecto desde la configuración JSON con múltiples direcciones
    static EffectPtr createFromConfig(const std::string& effectType, 
                                    const std::string& triggerType,
                                    CardPtr source, PlayerId owner,
                                    int value,
                                    const std::string& target,
                                    const std::vector<std::string>& directionStrings) {
        // Convertir strings de direcciones a Adjacency
        std::vector<GameMap::Adjacency> directions;
        for (const auto& dirStr : directionStrings) {
            if (dirStr == "up") {
                directions.push_back(GameMap::Adjacency::UP);
            } else if (dirStr == "down") {
                directions.push_back(GameMap::Adjacency::DOWN);
            } else if (dirStr == "top_left" || dirStr == "up_left") {
                directions.push_back(GameMap::Adjacency::TOP_LEFT);
            } else if (dirStr == "top_right" || dirStr == "up_right") {
                directions.push_back(GameMap::Adjacency::TOP_RIGHT);
            } else if (dirStr == "bottom_left" || dirStr == "down_left") {
                directions.push_back(GameMap::Adjacency::BOTTOM_LEFT);
            } else if (dirStr == "bottom_right" || dirStr == "down_right") {
                directions.push_back(GameMap::Adjacency::BOTTOM_RIGHT);
            }
        }
        // Si no hay direcciones, usar valor por defecto
        if (directions.empty()) {
            return createFromConfig(effectType, triggerType, source, owner, value, target);
        }
        // Aquí puedes agregar lógica para usar las direcciones si tu sistema lo requiere
        // Por ahora, solo llamamos al método legacy
        return createFromConfig(effectType, triggerType, source, owner, value, target);
    }
};
