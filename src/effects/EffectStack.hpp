#pragma once
#include <list>
#include <functional>
#include <array>
#include <cstdint>
#include <string>
#include <memory>
#include "../../libs/flat_hash_map.hpp"
#include "Effect.hpp"
#include "../game/GameMap.hpp"

enum class TriggerAction : uint8_t {
    ON_PLAY = 0,
    ON_MOVE = 1,
    ON_ATTACK = 2,
    ON_COMBAT = 3,
    ON_DRAW = 4,
    ON_END_OF_TURN = 5,
    ON_START_OF_TURN= 6,
    CAST_EFFECT = 7,
    MAX_ACTIONS = 8
};

// Stack to manage game effects
class EffectStack {
private:
    static constexpr int MAX_TRIGGERS = static_cast<int>(TriggerAction::MAX_ACTIONS);
    std::array<std::list<EffectPtr>, MAX_TRIGGERS> effectsByTrigger;
    ska::flat_hash_map<uint32_t, std::list<EffectPtr>::iterator> effectPointers;
    uint32_t nextEffectId = 1;

public:
    EffectStack() = default;
    
    // Add effect to stack
    uint32_t addEffect(EffectPtr effect, TriggerAction trigger) {
        uint32_t effectId = nextEffectId++;
        auto it = effectsByTrigger[static_cast<size_t>(trigger)].insert(
            effectsByTrigger[static_cast<size_t>(trigger)].end(), 
            effect
        );
        effectPointers[effectId] = it;
        return effectId;
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
    
    // Process all effects for a trigger
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
    
    // Convenience method to process end of turn effects
    void processEndOfTurn(GameMap& gameMap) {
        processTrigger(TriggerAction::ON_END_OF_TURN, gameMap, nullptr);
    }
    
    // Convenience method to process start of turn effects
    void processStartOfTurn(GameMap& gameMap) {
        processTrigger(TriggerAction::ON_START_OF_TURN, gameMap, nullptr);
    }
};
