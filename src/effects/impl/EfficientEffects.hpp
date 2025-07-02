#pragma once
#include "../Effect.hpp"
#include "../../game/GameMap.hpp"
#include "../../cards/Card.hpp"

// Example: Efficient adjacency buff that only affects cards adjacent to source
class AdjacentBuffEffect : public Effect {
private:
    uint8_t attackBonus;
    uint8_t healthBonus;

public:
    AdjacentBuffEffect(CardPtr src, PlayerId ownerId, const std::string& name, 
                      uint8_t atkBonus, uint8_t hpBonus)
        : Effect(src, ownerId, name), attackBonus(atkBonus), healthBonus(hpBonus) {}

    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)triggerCell; (void)targetCells;
        // Get source position directly - O(1) instead of O(n) search
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return true; // Keep effect if source not on map
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        MapCell* sourceCell = gameMap.at(sourceX, sourceY);
        if (!sourceCell) return true;
        
        // Apply buff to all adjacent friendly units - O(6) operations max
        for (int i = 0; i < 6; ++i) {
            MapCell* adjacentCell = gameMap.getNeighbor(static_cast<GameMap::Adjacency>(i), sourceCell, owner);
            if (!adjacentCell || !adjacentCell->card.has_value()) continue;
            
            auto adjacentCard = adjacentCell->card.value();
            if (adjacentCard->getOwner() != owner) continue; // Only buff friendly units
            
            if (auto adjacentUnit = std::dynamic_pointer_cast<Unit>(adjacentCard)) {
                // Apply buffs
                adjacentUnit->setAttack(adjacentUnit->getAttack() + attackBonus);
                adjacentUnit->setMaxHealth(adjacentUnit->getMaxHealth() + healthBonus);
                adjacentUnit->heal(healthBonus); // Heal by the bonus amount
            }
        }
        
        return true; // Keep the effect active
    }
    
    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)triggerCell; (void)affectedCells;
        // Get source position directly - O(1) instead of O(n) search
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return; // Keep effect if source not on map
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        MapCell* sourceCell = map.at(sourceX, sourceY);
        if (!sourceCell) return;
        
        // Apply buff to all adjacent friendly units - O(6) operations max
        for (int i = 0; i < 6; ++i) {
            MapCell* adjacentCell = map.getNeighbor(static_cast<GameMap::Adjacency>(i), sourceCell, owner);
            if (!adjacentCell || !adjacentCell->card.has_value()) continue;
            
            auto adjacentCard = adjacentCell->card.value();
            if (adjacentCard->getOwner() != owner) continue; // Only buff friendly units
            
            if (auto adjacentUnit = std::dynamic_pointer_cast<Unit>(adjacentCard)) {
                // Apply buffs
                adjacentUnit->setAttack(adjacentUnit->getAttack() + attackBonus);
                adjacentUnit->setMaxHealth(adjacentUnit->getMaxHealth() + healthBonus);
                adjacentUnit->heal(healthBonus); // Heal by the bonus amount
            }
        }
    }
};

// Example: Range-based effect that affects all units within range
class RangeBasedEffect : public Effect {
private:
    uint8_t effectRange;
    int8_t attackModifier; // Can be negative for debuff

public:
    RangeBasedEffect(CardPtr src, PlayerId ownerId, const std::string& name, 
                    uint8_t range, int8_t modifier)
        : Effect(src, ownerId, name), effectRange(range), attackModifier(modifier) {}

    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)triggerCell; (void)targetCells;
        // Get source coordinates directly - much more efficient!
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return true;
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        
        // Scan only cells within range instead of entire map
        uint8_t minX = (sourceX >= effectRange) ? sourceX - effectRange : 0;
        uint8_t maxX = std::min(static_cast<uint8_t>(sourceX + effectRange), static_cast<uint8_t>(4)); // MAP_WIDTH - 1
        uint8_t minY = (sourceY >= effectRange) ? sourceY - effectRange : 0;
        uint8_t maxY = std::min(static_cast<uint8_t>(sourceY + effectRange), static_cast<uint8_t>(6)); // MAP_HEIGHT - 1
        
        for (uint8_t x = minX; x <= maxX; ++x) {
            for (uint8_t y = minY; y <= maxY; ++y) {
                // Skip if distance exceeds our effect range
                uint8_t distance = std::abs(static_cast<int>(x - sourceX)) + std::abs(static_cast<int>(y - sourceY));
                if (distance > effectRange) continue;
                
                MapCell* cell = gameMap.at(x, y);
                if (!cell || !cell->card.has_value()) continue;
                
                auto card = cell->card.value();
                if (card->getOwner() == owner) continue; // Skip friendly units
                
                if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                    // Apply attack modifier
                    unit->setAttack(std::max(0, static_cast<int>(unit->getAttack()) + attackModifier));
                }
            }
        }
        
        return true; // Keep the effect active
    }
    
    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)triggerCell; (void)affectedCells;
        // Use the same optimized logic from checkEffect
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return;
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        
        // Only scan cells within range
        uint8_t minX = (sourceX >= effectRange) ? sourceX - effectRange : 0;
        uint8_t maxX = std::min(static_cast<uint8_t>(sourceX + effectRange), static_cast<uint8_t>(4));
        uint8_t minY = (sourceY >= effectRange) ? sourceY - effectRange : 0;
        uint8_t maxY = std::min(static_cast<uint8_t>(sourceY + effectRange), static_cast<uint8_t>(6));
        
        for (uint8_t x = minX; x <= maxX; ++x) {
            for (uint8_t y = minY; y <= maxY; ++y) {
                // Skip if distance exceeds our effect range
                uint8_t distance = std::abs(static_cast<int>(x - sourceX)) + std::abs(static_cast<int>(y - sourceY));
                if (distance > effectRange) continue;
                
                MapCell* cell = map.at(x, y);
                if (!cell || !cell->card.has_value()) continue;
                
                auto card = cell->card.value();
                if (card->getOwner() == owner) continue; // Skip friendly units
                
                if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                    // Apply attack modifier
                    unit->setAttack(std::max(0, static_cast<int>(unit->getAttack()) + attackModifier));
                }
            }
        }
    }
};
