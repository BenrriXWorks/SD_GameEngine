#pragma once
#include <array>
#include <ranges>
#include <functional>
#include <optional>
#include <numeric>
#include <string>

// Forward declaration
class MapCell;
#include "StrColors.hpp" // Necesario para el operador string()

class GameMap {

    public:
        enum class Adjacency : uint8_t {
            UP           = 0,
            TOP_RIGHT    = 1,
            BOTTOM_RIGHT = 2,
            DOWN         = 3,
            BOTTOM_LEFT  = 4,
            TOP_LEFT     = 5
        };
        static const std::string getAdjacencyName(Adjacency direction) {
            static const constexpr std::array<std::string, 6> ADJACENCY_NAMES = {
                "UP", "TOP_RIGHT", "BOTTOM_RIGHT", "DOWN", "BOTTOM_LEFT", "TOP_LEFT"
            };
            return ADJACENCY_NAMES[static_cast<uint8_t>(direction)];
        };

        static const constexpr uint8_t 
            MAP_HEIGHT = 7,
            MAP_WIDTH = 5;

    private:
        std::array<std::array<MapCell, MAP_WIDTH>, MAP_HEIGHT> mapData;

        static inline const auto& MAP_DEFAULT_TEMPLATE = [](){
            std::array<std::array<MapCell, MAP_WIDTH>, MAP_HEIGHT> mapDataTemplate;

            static constexpr auto isWalkable = [](uint8_t x, uint8_t y) -> bool { 
                return (y >= 3 && y < MAP_HEIGHT - 1) || 
                    x == MAP_WIDTH / 2 ||
                    (y == 2 && x >= 1 && x < MAP_WIDTH - 1) ;
            };

            using namespace std::ranges;
    
            for (const auto [x, y] : views::iota(0, MAP_WIDTH * MAP_HEIGHT) 
                | views::transform([](uint8_t i) { return std::pair{i % MAP_WIDTH, i / MAP_WIDTH}; })) 
                mapDataTemplate[y][x] = MapCell(
                    isWalkable(x, y) ? MapCell::FloorType::WALKABLE : MapCell::FloorType::NONE,
                    x, y);
            
            return mapDataTemplate;
        }();

    public:

        GameMap() : mapData(MAP_DEFAULT_TEMPLATE) {}
        
        MapCell* getNeighbor(Adjacency direction, const MapCell* cell) {
            if (!cell) return nullptr;

            static constexpr std::array positionTransformation = {
                std::pair{0, -1}, // UP
                std::pair{1,  0}, // TOP_RIGHT
                std::pair{1,  1}, // BOTTOM_RIGHT
                std::pair{0,  1}, // DOWN
                std::pair{-1, 1}, // BOTTOM_LEFT
                std::pair{-1, 0}  // TOP_LEFT
            };

            auto& [dx, dy] = positionTransformation[static_cast<uint8_t>(direction)];
            auto neighbor_x = dx + cell->x, neighbor_y = dy + cell->y;

            return neighbor_x >= MAP_WIDTH || neighbor_y >= MAP_HEIGHT ? 
                nullptr :
                &mapData[neighbor_y][neighbor_x];
        }

        inline MapCell* at(uint8_t x, uint8_t y) {
            return x >= MAP_WIDTH || y >= MAP_HEIGHT  ?  
                nullptr : // Out of bounds
                &mapData[y][x];
        }

        operator std::string() const {
            std::string result;
            result.reserve(MAP_HEIGHT * (MAP_WIDTH * 6 + 1));
            
            for (const auto& row : mapData) {
                for (const auto& cell : row) result += " " + std::string(cell) + " ";
                result += '\n';
            }
            using namespace StrColors;
            result += std::string() +
                "Legend:\n------\n" +
                "⬡: No floor\n" +
                MAGENTA + "⬣" + RESET + ": A Card and Effects are in the cell\n" +
                RED + "⬣" + RESET + ": A Card is in the cell\n" +
                BLUE + "⬣" + RESET + ": One or more effects are in the cell\n" +
                GREEN + "⬣" + RESET + ": Walkable floor without cards or effects.\n";
            return result;
        }
    

};