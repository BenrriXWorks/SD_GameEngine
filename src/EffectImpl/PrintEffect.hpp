#pragma once
#include "../Effect.hpp"
#include <print>
#include <string>

class PrintEffect : public Effect {
    public:
    
        PrintEffect(Card& src, const std::string& name) : Effect{src, name} {}

        bool check(GameMap& gameMap, MapCell* sourceCell, std::list<MapCell*> targetCells) override {
            std::println("{}", std::string(*this)); 
            return false;  // Se elimina tras activarse
        }
};
