#pragma once
#include <string>
#include <list>
#include <cstdint>

// Forward declarations
class GameMap;
class MapCell;

class Effect {
    public:
        using Card = uint8_t;
        Card& source;
        std::string name;

        Effect(Card& src, const std::string& n) : source(src), name(n) {}
        virtual ~Effect() = default;
        virtual bool check(GameMap& gameMap, MapCell* sourceCell, std::list<MapCell*> targetCells) = 0;

        operator std::string() const {
            return "Effect: " + name + " (Source Card: " + std::to_string(source) + ")";
        }
};