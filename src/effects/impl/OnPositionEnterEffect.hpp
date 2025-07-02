#pragma once
#include "../Effect.hpp"
#include "../../game/GameMap.hpp"
#include "../../cards/Card.hpp"

// Example: Effect that triggers when a card enters the position in front
class OnPositionEnterEffect : public Effect {
private:
    GameMap::Adjacency watchDirection;
    bool hasTarget = false;

public:
    OnPositionEnterEffect(CardPtr src, PlayerId ownerId, const std::string& name, 
                         GameMap::Adjacency direction)
        : Effect(src, ownerId, name), watchDirection(direction) {}

    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)targetCells;
        
        // Get source position efficiently - O(1)
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return true; // Keep the effect
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        MapCell* sourceCell = gameMap.at(sourceX, sourceY);
        if (!sourceCell) return true;
        
        // Get the watched cell
        MapCell* watchedCell = gameMap.getNeighbor(watchDirection, sourceCell, owner);
        if (!watchedCell) return true;
        
        // Check if this is a card entering (i.e., the trigger is exactly that cell)
        if (triggerCell && triggerCell == watchedCell && watchedCell->card.has_value()) {
            auto targetCard = watchedCell->card.value();
            
            // Only trigger on enemy units
            if (targetCard->getOwner() != owner) {
                hasTarget = true;
                return true; // Keep the effect and mark that we have a target
            }
        }
        
        // Reset target status if the cell is now empty
        if (hasTarget && (!watchedCell->card.has_value())) {
            hasTarget = false;
        }
        
        return true; // Keep the effect
    }
    
    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)affectedCells;
        
        if (!hasTarget) return; // Only apply if we have a valid target
        
        // Get source position efficiently
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return;
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        MapCell* sourceCell = map.at(sourceX, sourceY);
        if (!sourceCell) return;
        
        // Get the watched cell
        MapCell* watchedCell = map.getNeighbor(watchDirection, sourceCell, owner);
        if (!watchedCell || !watchedCell->card.has_value()) return;
        
        // Apply effect - example: deal 1 damage to the unit that entered
        auto targetCard = watchedCell->card.value();
        if (targetCard->getOwner() != owner) { // Enemy unit
            if (auto targetUnit = std::dynamic_pointer_cast<Unit>(targetCard)) {
                // Apply effect: deal 1 damage
                targetUnit->takeDamage(1);
                
                // Reset if the unit died or moved
                if (targetUnit->getHealth() == 0 || watchedCell != triggerCell) {
                    hasTarget = false;
                }
            }
        }
    }
};
