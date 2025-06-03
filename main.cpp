#include <print>
#include <array>
#include <functional>
#include <utility>
#include <string>
#include "src/MapCell.hpp"
#include "src/GameMap.hpp"


using namespace std;


int main() {

    srand(static_cast<unsigned int>(time(nullptr))); // Inicializar generador de números aleatorios

    // Inicializar mapa
    GameMap gameMap;
    uint8_t x = rand() % GameMap::MAP_WIDTH, y = rand() % GameMap::MAP_HEIGHT;

    // Obtener una celda cualquiera
    auto cell = gameMap.at(x, y);
    if (cell == nullptr || gameMap.at(x,y)->floor == MapCell::FloorType::NONE) 
        return (println("Player would be put out of bounds (x:{}, y:{})", x, y), 1);
    println("Cell at (x:{}, y:{}): {}", cell->x, cell->y, (string)(*cell));
    
    // Asignar una carta stub (un numero) de ejemplo a la celda inicial
    if (!cell->card.has_value()) cell->card = 1; 
    

    // Agregar efectos pasivos a la celdas random.
    for (int i = 0; i < 10; ++i) {
        auto randCell = gameMap.at(rand() % GameMap::MAP_WIDTH, rand() % GameMap::MAP_HEIGHT);
        if (!randCell) {
            println("Random cell is out of bounds");
            continue;
        } else if (randCell->floor == MapCell::FloorType::NONE) {
            println("Random cell at (x:{}, y:{}) is not walkable therefore cant have effect", randCell->x, randCell->y);
            continue;
        }
        randCell->passiveEffects.push_back([](MapCell& c) { c.card = 2; }); // Asignar un efecto de ejemplo
        randCell->passiveEffects.push_back([](MapCell& c) { c.floor = MapCell::FloorType::WALKABLE; }); // Cambiar el piso
        println("Added effects to random cell at (x:{}, y:{}): {}", randCell->x, randCell->y, string(*randCell));
    }

    // Imprimir el mapa
    println("========= MAP =========");
    println("{}", string(gameMap));
    println("=======================");


    
    // Imprimir vecinos de la celda
    println("Neighbors of cell at (x:{}, y:{}):", cell->x, cell->y);
    for (auto direction : std::array{
        GameMap::Adjacency::UP,
        GameMap::Adjacency::TOP_RIGHT,
        GameMap::Adjacency::BOTTOM_RIGHT,
        GameMap::Adjacency::DOWN,
        GameMap::Adjacency::BOTTOM_LEFT,
        GameMap::Adjacency::TOP_LEFT
    }) {
        if (auto* neighbor = gameMap.getNeighbor(direction, cell)) 
            println("Neighbor {}: {}", gameMap.getAdjacencyName(direction), string(*neighbor));
        else
            println("No neighbor {} (out of bounds)", static_cast<int>(direction));
    }

    return 0;
}