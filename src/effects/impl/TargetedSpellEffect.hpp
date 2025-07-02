#pragma once
#include "../Effect.hpp"
#include "../../game/GameMap.hpp"
#include "../../cards/Card.hpp"

// Example: Spell effect that targets a specific position permanently
class TargetedSpellEffect : public Effect {
private:
    uint8_t targetX, targetY;
    uint8_t damageAmount;
    bool isInstant;

public:
    TargetedSpellEffect(CardPtr src, PlayerId ownerId, const std::string& name, 
                       uint8_t x, uint8_t y, uint8_t damage, bool instant = true)
        : Effect(src, ownerId, name), targetX(x), targetY(y), damageAmount(damage), isInstant(instant) {}

    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)triggerCell; (void)targetCells;
        
        // Check if target position is valid
        MapCell* targetCell = gameMap.at(targetX, targetY);
        if (!targetCell || !targetCell->card.has_value()) {
            return !isInstant; // Keep if persistent, remove if instant
        }
        
        // Apply effect to the card at target position
        auto targetCard = targetCell->card.value();
        if (auto targetUnit = std::dynamic_pointer_cast<Unit>(targetCard)) {
            // Apply damage
            targetUnit->takeDamage(damageAmount);
        }
        
        return !isInstant; // If instant, remove after activation; otherwise, keep
    }
    
    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)triggerCell; (void)affectedCells;
        
        // Check if target position is valid
        MapCell* targetCell = map.at(targetX, targetY);
        if (!targetCell || !targetCell->card.has_value()) return;
        
        // Apply effect to the card at target position
        auto targetCard = targetCell->card.value();
        if (auto targetUnit = std::dynamic_pointer_cast<Unit>(targetCard)) {
            // Apply damage
            targetUnit->takeDamage(damageAmount);
        }
    }
};
