#pragma once
#include <string>
#include <list>
#include <cstdint>
#include <memory>
#include "../utils/Types.hpp"
#include "../cards/Card.hpp"

// Forward declarations needed for the method signatures
class GameMap;
class MapCell;

class Effect {
    public:
        CardPtr source;
        PlayerId owner;
        std::string name;

        Effect(CardPtr src, PlayerId ownerId, const std::string& n) : source(src), owner(ownerId), name(n) {}
        virtual ~Effect() = default;
        
        // Pure virtual method for effect logic - receives gameMap to find source position
        virtual bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) = 0;
        
        // Pure virtual method for applying the effect
        virtual void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) = 0;
        
        // Final check method that includes source validation
        bool check(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) {
            // Check if source card still exists and is alive (for Unit cards)
            if (!source) return false;
            
            // For Unit cards, check if they're still alive
            if (auto unit = std::dynamic_pointer_cast<Unit>(source)) {
                if (!unit->isAlive()) return false;
            }
            
            // Call the specific effect logic
            return checkEffect(gameMap, triggerCell, targetCells);
        }

        // Declaration only - implementation in EffectImpl.hpp
        operator std::string() const;
};
