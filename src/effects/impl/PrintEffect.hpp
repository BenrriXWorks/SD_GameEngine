#pragma once
#include "../Effect.hpp"
#include <print>
#include <string>

class PrintEffect : public Effect {
    public:
    
        PrintEffect(CardPtr src, PlayerId ownerId, const std::string& name) : Effect{src, ownerId, name} {}

        bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
            (void)gameMap; (void)triggerCell; (void)targetCells;
            std::println("{}", std::string(*this)); 
            return false;  // Se elimina tras activarse
        }
};
