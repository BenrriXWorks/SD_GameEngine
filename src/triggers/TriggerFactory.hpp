#pragma once
#include "Trigger.hpp"
#include "TriggerImpl.hpp"
#include "../game/GameMap.hpp"
#include <memory>
#include <vector>
#include <string>

class TriggerFactory {
public:
    // Crear trigger de tipo OnPlay
    static TriggerPtr createOnPlayTrigger(CardPtr source, PlayerId owner) {
        return std::make_shared<OnPlayTrigger>(source, owner);
    }
    
    // Crear trigger de tipo OnCast
    static TriggerPtr createOnCastTrigger(CardPtr source, PlayerId owner) {
        return std::make_shared<OnCastTrigger>(source, owner);
    }
    
    // Crear trigger de tipo OnStartTurn
    static TriggerPtr createOnStartTurnTrigger(CardPtr source, PlayerId owner) {
        return std::make_shared<OnStartTurnTrigger>(source, owner);
    }
    
    // Crear trigger de tipo OnEndTurn
    static TriggerPtr createOnEndTurnTrigger(CardPtr source, PlayerId owner) {
        return std::make_shared<OnEndTurnTrigger>(source, owner);
    }
    
    // Crear trigger de tipo OnEnemyEnter (una dirección)
    static TriggerPtr createOnEnemyEnterTrigger(CardPtr source, PlayerId owner, 
                                              GameMap::Adjacency direction) {
        return std::make_shared<OnEnemyEnterTrigger>(source, owner, direction);
    }
    
    // Crear trigger de tipo OnEnemyEnter (múltiples direcciones)
    static TriggerPtr createOnEnemyEnterTrigger(CardPtr source, PlayerId owner, 
                                              const std::vector<GameMap::Adjacency>& directions) {
        return std::make_shared<OnEnemyEnterTrigger>(source, owner, directions);
    }
    
    // Crear trigger de tipo OnEnemyExit (una dirección)
    static TriggerPtr createOnEnemyExitTrigger(CardPtr source, PlayerId owner, 
                                             GameMap::Adjacency direction) {
        return std::make_shared<OnEnemyExitTrigger>(source, owner, direction);
    }
    
    // Crear trigger de tipo OnEnemyExit (múltiples direcciones)
    static TriggerPtr createOnEnemyExitTrigger(CardPtr source, PlayerId owner, 
                                             const std::vector<GameMap::Adjacency>& directions) {
        return std::make_shared<OnEnemyExitTrigger>(source, owner, directions);
    }
    
    // Crear trigger de tipo OnAllyEnter (una dirección)
    static TriggerPtr createOnAllyEnterTrigger(CardPtr source, PlayerId owner, 
                                             GameMap::Adjacency direction) {
        return std::make_shared<OnAllyEnterTrigger>(source, owner, direction);
    }
    
    // Crear trigger de tipo OnAllyEnter (múltiples direcciones)
    static TriggerPtr createOnAllyEnterTrigger(CardPtr source, PlayerId owner, 
                                             const std::vector<GameMap::Adjacency>& directions) {
        return std::make_shared<OnAllyEnterTrigger>(source, owner, directions);
    }
    
    // Crear trigger de tipo OnAttack (mapear temporalmente a OnPlay)
    static TriggerPtr createOnAttackTrigger(CardPtr source, PlayerId owner) {
        return std::make_shared<OnPlayTrigger>(source, owner);
    }
    
    // Crear trigger de tipo OnAttacked (mapear temporalmente a OnPlay)
    static TriggerPtr createOnAttackedTrigger(CardPtr source, PlayerId owner) {
        return std::make_shared<OnPlayTrigger>(source, owner);
    }
    
    // Crear trigger de tipo OnKill (mapear temporalmente a OnPlay)
    static TriggerPtr createOnKillTrigger(CardPtr source, PlayerId owner) {
        return std::make_shared<OnPlayTrigger>(source, owner);
    }
    
    // Crear trigger de tipo OnDeath (mapear temporalmente a OnPlay)
    static TriggerPtr createOnDeathTrigger(CardPtr source, PlayerId owner) {
        return std::make_shared<OnPlayTrigger>(source, owner);
    }
    
    // Método para crear un trigger a partir de un string
    static TriggerPtr createFromString(const std::string& triggerType, CardPtr source, 
                                      PlayerId owner, GameMap::Adjacency direction) {
        if (triggerType == "on_play") {
            return createOnPlayTrigger(source, owner);
        } else if (triggerType == "on_cast") {
            return createOnCastTrigger(source, owner);
        } else if (triggerType == "on_start_turn") {
            return createOnStartTurnTrigger(source, owner);
        } else if (triggerType == "on_end_turn") {
            return createOnEndTurnTrigger(source, owner);
        } else if (triggerType == "on_enemy_enter") {
            return createOnEnemyEnterTrigger(source, owner, direction);
        } else if (triggerType == "on_enemy_exit") {
            return createOnEnemyExitTrigger(source, owner, direction);
        } else if (triggerType == "on_ally_enter") {
            return createOnAllyEnterTrigger(source, owner, direction);
        }
        
        // Trigger por defecto (en caso de error)
        return nullptr;
    }
    
    // Método para crear un trigger a partir de un string (con múltiples direcciones)
    static TriggerPtr createFromString(const std::string& triggerType, CardPtr source, 
                                      PlayerId owner, const std::vector<GameMap::Adjacency>& directions) {
        if (triggerType == "on_enemy_enter") {
            return createOnEnemyEnterTrigger(source, owner, directions);
        } else if (triggerType == "on_enemy_exit") {
            return createOnEnemyExitTrigger(source, owner, directions);
        } else if (triggerType == "on_ally_enter") {
            return createOnAllyEnterTrigger(source, owner, directions);
        } else {
            // Para triggers que no usan direcciones, usar la primera dirección (o una dirección por defecto)
            GameMap::Adjacency direction = directions.empty() ? GameMap::Adjacency::UP : directions[0];
            return createFromString(triggerType, source, owner, direction);
        }
    }
};
