#pragma once
#include "../Effect.hpp"
#include "../../game/GameMap.hpp"
#include "../../cards/Card.hpp"

// Example: Effect that reduces attack of the card in front by 1
class PositionalDebuffEffect : public Effect {
private:
    GameMap::Adjacency targetDirection;
    uint8_t attackReduction;

public:
    PositionalDebuffEffect(CardPtr src, PlayerId ownerId, const std::string& name, 
                          GameMap::Adjacency direction, uint8_t reduction = 1)
        : Effect(src, ownerId, name), targetDirection(direction), attackReduction(reduction) {}

    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)triggerCell; (void)targetCells;
        
        // Get source position efficiently - O(1)
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return true; // Keep the effect
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        MapCell* sourceCell = gameMap.at(sourceX, sourceY);
        if (!sourceCell) return true;
        
        // Get the target cell in the specified direction
        MapCell* targetCell = gameMap.getNeighbor(targetDirection, sourceCell, owner);
        if (!targetCell || !targetCell->card.has_value()) return true;
        
        // Apply effect to enemy unit
        auto targetCard = targetCell->card.value();
        if (targetCard->getOwner() != owner) { // Only affect enemy units
            if (auto targetUnit = std::dynamic_pointer_cast<Unit>(targetCard)) {
                // Apply attack reduction
                uint8_t currentAttack = targetUnit->getAttack();
                targetUnit->setAttack(std::max(0, static_cast<int>(currentAttack) - attackReduction));
            }
        }
        
        return true; // Keep the effect active
    }
    
    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)triggerCell; (void)affectedCells;
        
        // Get source position efficiently
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return;
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        MapCell* sourceCell = map.at(sourceX, sourceY);
        if (!sourceCell) return;
        
        // Get the target cell
        MapCell* targetCell = map.getNeighbor(targetDirection, sourceCell, owner);
        if (!targetCell || !targetCell->card.has_value()) return;
        
        // Apply effect to enemy unit
        auto targetCard = targetCell->card.value();
        if (targetCard->getOwner() != owner) { // Only affect enemy units
            if (auto targetUnit = std::dynamic_pointer_cast<Unit>(targetCard)) {
                // Apply attack reduction
                uint8_t currentAttack = targetUnit->getAttack();
                targetUnit->setAttack(std::max(0, static_cast<int>(currentAttack) - attackReduction));
            }
        }
    }
};
