#pragma once
#include <array>
#include <ranges>
#include <functional>
#include <optional>
#include <numeric>
#include <string>
#include "MapCell.hpp"  // Include completo en lugar de forward declaration
#include "../utils/StrColors.hpp" // Necesario para el operador string()
#include "../utils/Types.hpp"

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
        
        // Invierte la dirección para el jugador 1 (reflejo vertical)
        static Adjacency invertDirection(Adjacency direction) {
            static constexpr std::array<Adjacency, 6> INVERTED_DIRECTIONS = {
                Adjacency::DOWN,         // UP -> DOWN
                Adjacency::BOTTOM_RIGHT, // TOP_RIGHT -> BOTTOM_RIGHT
                Adjacency::TOP_RIGHT,    // BOTTOM_RIGHT -> TOP_RIGHT
                Adjacency::UP,           // DOWN -> UP
                Adjacency::TOP_LEFT,     // BOTTOM_LEFT -> TOP_LEFT
                Adjacency::BOTTOM_LEFT   // TOP_LEFT -> BOTTOM_LEFT
            };
            return INVERTED_DIRECTIONS[static_cast<uint8_t>(direction)];
        }
        
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
            
            static constexpr auto isSpawn = [](uint8_t x, uint8_t y) -> bool {
                return (x == 2 && y == 0) || (x == 2 && y == 6); // Spawn positions
            };

            using namespace std::ranges;
    
            for (const auto [x, y] : views::iota(0, MAP_WIDTH * MAP_HEIGHT) 
                | views::transform([](uint8_t i) { return std::pair{i % MAP_WIDTH, i / MAP_WIDTH}; })) {
                MapCell::FloorType floorType;
                if (isSpawn(x, y)) {
                    floorType = MapCell::FloorType::SPAWN;
                } else if (isWalkable(x, y)) {
                    floorType = MapCell::FloorType::WALKABLE;
                } else {
                    floorType = MapCell::FloorType::NONE;
                }
                mapDataTemplate[y][x] = MapCell(floorType);
            }
            
            return mapDataTemplate;
        }();

    public:

        GameMap() : mapData(MAP_DEFAULT_TEMPLATE) {}
        
        MapCell* getNeighbor(Adjacency direction, const MapCell* cell, PlayerId playerId = 0) {
            if (!cell) return nullptr;

            // Invertir dirección para el jugador 1
            if (playerId == 1) {
                direction = invertDirection(direction);
            }

            auto [cellX, cellY] = getCellCoordinates(cell);
            if (cellX == 255) return nullptr; // Cell not found

            static constexpr std::array positionTransformation = {
                std::pair{0, -1}, // UP
                std::pair{1,  0}, // TOP_RIGHT
                std::pair{1,  1}, // BOTTOM_RIGHT
                std::pair{0,  1}, // DOWN
                std::pair{-1, 1}, // BOTTOM_LEFT
                std::pair{-1, 0}  // TOP_LEFT
            };

            auto& [dx, dy] = positionTransformation[static_cast<uint8_t>(direction)];
            auto neighbor_x = dx + cellX, neighbor_y = dy + cellY;

            return neighbor_x >= MAP_WIDTH || neighbor_y >= MAP_HEIGHT ? 
                nullptr :
                &mapData[neighbor_y][neighbor_x];
        }

        inline MapCell* at(uint8_t x, uint8_t y) {
            return x >= MAP_WIDTH || y >= MAP_HEIGHT  ?  
                nullptr : // Out of bounds
                &mapData[y][x];
        }

        inline const MapCell* at(uint8_t x, uint8_t y) const {
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
                RED + "⬣" + RESET + ": A Card is in the cell\n" +
                YELLOW + "⬣" + RESET + ": Spawn position for legends\n" +
                GREEN + "⬣" + RESET + ": Walkable floor without cards.\n";
            return result;
        }
    
        // Dimension accessors
        constexpr uint8_t getWidth() const { return MAP_WIDTH; }
        constexpr uint8_t getHeight() const { return MAP_HEIGHT; }
        
        // Helper para encontrar posiciones de spawn
        std::pair<uint8_t, uint8_t> getSpawnPosition(PlayerId playerId) const {
            // Player 0 spawn en (2,0), Player 1 spawn en (2,6)
            if (playerId == 0) return {2, 0};
            if (playerId == 1) return {2, 6};
            return {255, 255}; // Invalid position for other players
        }
        
        bool isSpawnPosition(uint8_t x, uint8_t y) const {
            const MapCell* cell = at(x, y);
            return cell && cell->isSpawn();
        }
    
        // Helper to find coordinates of a cell in the map
        std::pair<uint8_t, uint8_t> getCellCoordinates(const MapCell* cell) const {
            if (!cell) return {255, 255};
            
            for (uint8_t y = 0; y < MAP_HEIGHT; ++y) {
                for (uint8_t x = 0; x < MAP_WIDTH; ++x) {
                    if (&mapData[y][x] == cell) {
                        return {x, y};
                    }
                }
            }
            return {255, 255}; // Not found
        }

};
