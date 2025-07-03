#pragma once
#include <list>
#include <functional>
#include <array>
#include <cstdint>
#include <string>
#include <memory>
#include "../../libs/flat_hash_map.hpp"
#include "Effect.hpp"
#include "ComposedEffect.hpp"
#include "../game/GameMap.hpp"

// Forward declaration
class GameState;

// Trigger actions for the effect stack - Mantenemos compatibilidad con sistema legacy
enum class TriggerAction : uint8_t {
    ON_PLAY = 0,           // Cuando se juega una carta
    ON_MOVE = 1,           // Cuando una unidad se mueve
    ON_ATTACK = 2,         // Cuando una unidad ataca
    ON_COMBAT = 3,         // Durante el combate
    ON_DRAW = 4,           // Cuando se roba una carta
    ON_END_OF_TURN = 5,    // Al final del turno
    ON_START_OF_TURN= 6,   // Al inicio del turno
    CAST_EFFECT = 7,       // Al lanzar un hechizo
    ON_POSITION_ENTER = 8, // Cuando una unidad entra en una posición
    ON_POSITION_EXIT = 9,  // Cuando una unidad sale de una posición
    MAX_ACTIONS = 10
};

// Stack to manage game effects - Ahora compatible con ComposedEffect y sistema legacy
class EffectStack {
private:
    static constexpr int MAX_TRIGGERS = static_cast<int>(TriggerAction::MAX_ACTIONS);
    std::array<std::list<EffectPtr>, MAX_TRIGGERS> effectsByTrigger;
    ska::flat_hash_map<uint32_t, std::list<EffectPtr>::iterator> effectPointers;
    uint32_t nextEffectId = 1;

public:
    EffectStack() = default;
    
    // Add effect to stack (legacy compatibility)
    uint32_t addEffect(EffectPtr effect, TriggerAction trigger) {
        uint32_t effectId = nextEffectId++;
        auto it = effectsByTrigger[static_cast<size_t>(trigger)].insert(
            effectsByTrigger[static_cast<size_t>(trigger)].end(), 
            effect
        );
        effectPointers[effectId] = it;
        return effectId;
    }
    
    // Add ComposedEffect - el sistema lo manejará automáticamente basado en sus triggers internos
    uint32_t addComposedEffect(std::shared_ptr<ComposedEffect> composedEffect) {
        // Para efectos compuestos, podemos inferir el trigger principal o usar ON_PLAY por defecto
        // En una implementación más avanzada, el ComposedEffect podría tener múltiples registros
        return addEffect(composedEffect, TriggerAction::ON_PLAY);
    }
    
    // Overload for default trigger (ON_PLAY)
    uint32_t addEffect(EffectPtr effect) {
        return addEffect(effect, TriggerAction::ON_PLAY);
    }
    
    // Remove effect from stack
    void removeEffect(uint32_t effectId) {
        auto it = effectPointers.find(effectId);
        if (it != effectPointers.end()) {
            auto effectIt = it->second;
            // Find which trigger list contains this effect
            for (size_t i = 0; i < effectsByTrigger.size(); ++i) {
                auto& list = effectsByTrigger[i];
                // Check if the iterator belongs to this list by comparing with all elements
                bool found = false;
                for (auto checkIt = list.begin(); checkIt != list.end(); ++checkIt) {
                    if (checkIt == effectIt) {
                        found = true;
                        break;
                    }
                }
                
                if (found) {
                    list.erase(effectIt);
                    break;
                }
            }
            effectPointers.erase(it);
        }
    }
    
    // Process all effects for a trigger - Compatible con ComposedEffect y efectos legacy
    void processTrigger(TriggerAction trigger, GameMap& gameMap, MapCell* triggerCell, 
                       const std::list<MapCell*>& affectedCells = {}) {
        auto& effects = effectsByTrigger[static_cast<size_t>(trigger)];
        
        // Use a safe iteration approach since effects may be removed during processing
        auto it = effects.begin();
        while (it != effects.end()) {
            auto effect = *it;
            bool keepEffect = effect->checkEffect(gameMap, triggerCell, affectedCells);
            
            if (keepEffect) {
                effect->apply(gameMap, triggerCell, affectedCells);
                
                // Para ComposedEffect, también procesar efectos de estado de juego si es necesario
                if (auto composedEffect = std::dynamic_pointer_cast<ComposedEffect>(effect)) {
                    // En el futuro, podríamos pasar GameState aquí
                    // composedEffect->applyToGameState(gameMap, triggerCell, gameState);
                }
                
                ++it;
            } else {
                // Find and remove the effectId associated with this effect
                for (auto pointerIt = effectPointers.begin(); pointerIt != effectPointers.end(); ++pointerIt) {
                    if (pointerIt->second == it) {
                        effectPointers.erase(pointerIt);
                        break;
                    }
                }
                it = effects.erase(it);
            }
        }
    }
    
    // Versión extendida que incluye GameState para efectos compuestos
    void processTriggerWithGameState(TriggerAction trigger, GameMap& gameMap, MapCell* triggerCell, 
                                   GameState* gameState, const std::list<MapCell*>& affectedCells = {}) {
        auto& effects = effectsByTrigger[static_cast<size_t>(trigger)];
        
        auto it = effects.begin();
        while (it != effects.end()) {
            auto effect = *it;
            bool keepEffect = effect->checkEffect(gameMap, triggerCell, affectedCells);
            
            if (keepEffect) {
                effect->apply(gameMap, triggerCell, affectedCells);
                
                // Para ComposedEffect, también procesar efectos de estado de juego
                if (auto composedEffect = std::dynamic_pointer_cast<ComposedEffect>(effect)) {
                    composedEffect->applyToGameState(gameMap, triggerCell, gameState);
                }
                
                ++it;
            } else {
                // Find and remove the effectId associated with this effect
                for (auto pointerIt = effectPointers.begin(); pointerIt != effectPointers.end(); ++pointerIt) {
                    if (pointerIt->second == it) {
                        effectPointers.erase(pointerIt);
                        break;
                    }
                }
                it = effects.erase(it);
            }
        }
    }
    
    // Convenience methods - Legacy compatibility
    void processEndOfTurn(GameMap& gameMap) {
        processTrigger(TriggerAction::ON_END_OF_TURN, gameMap, nullptr);
    }
    
    void processStartOfTurn(GameMap& gameMap) {
        processTrigger(TriggerAction::ON_START_OF_TURN, gameMap, nullptr);
    }
    
    // Convenience methods with GameState support
    void processEndOfTurn(GameMap& gameMap, GameState* gameState) {
        processTriggerWithGameState(TriggerAction::ON_END_OF_TURN, gameMap, nullptr, gameState);
    }
    
    void processStartOfTurn(GameMap& gameMap, GameState* gameState) {
        processTriggerWithGameState(TriggerAction::ON_START_OF_TURN, gameMap, nullptr, gameState);
    }
    
    void processOnPlay(GameMap& gameMap, MapCell* triggerCell, GameState* gameState) {
        processTriggerWithGameState(TriggerAction::ON_PLAY, gameMap, triggerCell, gameState);
    }
    
    void processOnCast(GameMap& gameMap, MapCell* triggerCell, GameState* gameState) {
        processTriggerWithGameState(TriggerAction::CAST_EFFECT, gameMap, triggerCell, gameState);
    }
    
    // Método para procesar efectos cuando una unidad entra en una posición
    void processPositionEnter(GameMap& gameMap, MapCell* triggerCell, GameState* gameState = nullptr) {
        if (gameState) {
            processTriggerWithGameState(TriggerAction::ON_POSITION_ENTER, gameMap, triggerCell, gameState);
        } else {
            processTrigger(TriggerAction::ON_POSITION_ENTER, gameMap, triggerCell);
        }
    }
    
    // Obtener número de efectos por trigger (para depuración)
    size_t getEffectCount(TriggerAction trigger) const {
        return effectsByTrigger[static_cast<size_t>(trigger)].size();
    }
    
    // Limpiar todos los efectos
    void clear() {
        for (auto& list : effectsByTrigger) {
            list.clear();
        }
        effectPointers.clear();
        nextEffectId = 1;
    }
};
