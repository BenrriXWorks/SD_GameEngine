#pragma once
#include "TargetSelector.hpp"
#include "../game/GameMap.hpp"

// Selector que afecta solo a la carta fuente
class SelfTargetSelector : public TargetSelector {
public:
    SelfTargetSelector(CardPtr src, PlayerId ownerId)
        : TargetSelector(src, ownerId, "Self") {}
    
    std::list<MapCell*> selectTargets(GameMap& gameMap, MapCell* triggerCell) override {
        (void)triggerCell; // Suprimir warning de parámetro no usado
        std::list<MapCell*> targets;
        
        // Solo afecta a la carta fuente si está en el mapa
        if (auto sourceUnit = std::dynamic_pointer_cast<Unit>(source)) {
            if (sourceUnit->isOnMap()) {
                auto [x, y] = sourceUnit->getCoordinates();
                MapCell* sourceCell = gameMap.at(x, y);
                if (sourceCell && sourceCell->card.has_value()) {
                    targets.push_back(sourceCell);
                }
            }
        }
        
        return targets;
    }
};

// Selector que afecta cartas adyacentes
class AdjacentTargetSelector : public TargetSelector {
public:
    enum class FilterType {
        ALL,            // Todas las cartas adyacentes
        ALLIES_ONLY,    // Solo cartas aliadas
        ENEMIES_ONLY,   // Solo cartas enemigas (sin contar líderes)
        NON_LEADERS     // Todas excepto líderes
    };

private:
    FilterType filter;

public:
    AdjacentTargetSelector(CardPtr src, PlayerId ownerId, FilterType filterType = FilterType::ALL)
        : TargetSelector(src, ownerId, "Adjacent"), filter(filterType) {}
    
    std::list<MapCell*> selectTargets(GameMap& gameMap, MapCell* triggerCell) override {
        (void)triggerCell; // Suprimir warning de parámetro no usado
        std::list<MapCell*> targets;
        
        // Obtener posición de la carta fuente
        if (auto sourceUnit = std::dynamic_pointer_cast<Unit>(source)) {
            if (sourceUnit->isOnMap()) {
                auto [x, y] = sourceUnit->getCoordinates();
                
                // Calcular celdas adyacentes usando las direcciones
                std::vector<std::pair<int, int>> adjacentOffsets = {
                    {-1, -1}, {-1, 0}, {-1, 1},  // arriba-izq, arriba, arriba-der
                    { 0, -1},          { 0, 1},  // izquierda, derecha
                    { 1, -1}, { 1, 0}, { 1, 1}   // abajo-izq, abajo, abajo-der
                };
                
                for (auto [dx, dy] : adjacentOffsets) {
                    int newX = x + dx;
                    int newY = y + dy;
                    // Verificar límites del mapa
                    if (newX >= 0 && newX < GameMap::MAP_WIDTH && newY >= 0 && newY < GameMap::MAP_HEIGHT) {
                        auto cell = gameMap.at(static_cast<uint8_t>(newX), static_cast<uint8_t>(newY));
                        if (cell && isValidTarget(cell)) {
                            targets.push_back(cell);
                        }
                    }
                }
            }
        }
        
        return targets;
    }
    
    bool isValidTarget(MapCell* cell) const override {
        if (!cell || !cell->card.has_value()) return false;
        
        auto card = cell->card.value();
        
        switch (filter) {
            case FilterType::ALL:
                return true;
                
            case FilterType::ALLIES_ONLY:
                return card->getOwner() == owner;
                
            case FilterType::ENEMIES_ONLY:
                // Solo enemigos, sin contar líderes
                return card->getOwner() != owner && 
                       std::dynamic_pointer_cast<Legend>(card) == nullptr;
                
            case FilterType::NON_LEADERS:
                // Todas las cartas excepto líderes
                return std::dynamic_pointer_cast<Legend>(card) == nullptr;
        }
        
        return false;
    }
};

// Selector que afecta todas las cartas de un tipo en el mapa
class AllCardsTargetSelector : public TargetSelector {
public:
    enum class FilterType {
        ALL_ALLIES,     // Todas las cartas aliadas
        ALL_ENEMIES,    // Todas las cartas enemigas (sin líderes)
        ALL_UNITS,      // Todas las unidades (sin líderes)
        ALL_CARDS       // Absolutamente todas las cartas
    };

private:
    FilterType filter;

public:
    AllCardsTargetSelector(CardPtr src, PlayerId ownerId, FilterType filterType)
        : TargetSelector(src, ownerId, "AllCards"), filter(filterType) {}
    
    std::list<MapCell*> selectTargets(GameMap& gameMap, MapCell* triggerCell) override {
        (void)triggerCell; // Suppress unused parameter warning
        std::list<MapCell*> targets;
        
        // Recorrer todo el mapa
        for (uint8_t y = 0; y < gameMap.getHeight(); y++) {
            for (uint8_t x = 0; x < gameMap.getWidth(); x++) {
                MapCell* cell = gameMap.at(x, y);
                if (isValidTarget(cell)) {
                    targets.push_back(cell);
                }
            }
        }
        
        return targets;
    }
    
    bool isValidTarget(MapCell* cell) const override {
        if (!cell || !cell->card.has_value()) return false;
        
        auto card = cell->card.value();
        
        switch (filter) {
            case FilterType::ALL_ALLIES:
                return card->getOwner() == owner;
                
            case FilterType::ALL_ENEMIES:
                // Solo enemigos, sin contar líderes
                return card->getOwner() != owner && 
                       std::dynamic_pointer_cast<Legend>(card) == nullptr;
                
            case FilterType::ALL_UNITS:
                // Todas las unidades (sin líderes)
                return std::dynamic_pointer_cast<Unit>(card) != nullptr &&
                       std::dynamic_pointer_cast<Legend>(card) == nullptr;
                
            case FilterType::ALL_CARDS:
                return true;
        }
        
        return false;
    }
};

// Selector que afecta una posición específica
class SpecificPositionTargetSelector : public TargetSelector {
private:
    uint8_t targetX, targetY;

public:
    SpecificPositionTargetSelector(CardPtr src, PlayerId ownerId, uint8_t x, uint8_t y)
        : TargetSelector(src, ownerId, "SpecificPosition"), targetX(x), targetY(y) {}
    
    std::list<MapCell*> selectTargets(GameMap& gameMap, MapCell* triggerCell) override {
        (void)triggerCell; // Suppress unused parameter warning
        std::list<MapCell*> targets;
        
        MapCell* targetCell = gameMap.at(targetX, targetY);
        if (targetCell) {
            targets.push_back(targetCell);
        }
        
        return targets;
    }
};

// Selector que afecta una dirección específica desde la carta fuente
class DirectionalTargetSelector : public TargetSelector {
private:
    GameMap::Adjacency direction;
    AdjacentTargetSelector::FilterType filter;

public:
    DirectionalTargetSelector(CardPtr src, PlayerId ownerId, GameMap::Adjacency dir, 
                            AdjacentTargetSelector::FilterType filterType = AdjacentTargetSelector::FilterType::ALL)
        : TargetSelector(src, ownerId, "Directional"), direction(dir), filter(filterType) {}
    
    std::list<MapCell*> selectTargets(GameMap& gameMap, MapCell* triggerCell) override {
        (void)triggerCell; // Suppress unused parameter warning
        std::list<MapCell*> targets;
        
        // Obtener posición de la carta fuente
        if (auto sourceUnit = std::dynamic_pointer_cast<Unit>(source)) {
            if (sourceUnit->isOnMap()) {
                auto [x, y] = sourceUnit->getCoordinates();
                MapCell* sourceCell = gameMap.at(x, y);
                
                if (sourceCell) {
                    MapCell* targetCell = gameMap.getNeighbor(direction, sourceCell, owner);
                    if (isValidTarget(targetCell)) {
                        targets.push_back(targetCell);
                    }
                }
            }
        }
        
        return targets;
    }
    
    bool isValidTarget(MapCell* cell) const override {
        if (!cell || !cell->card.has_value()) return false;
        
        auto card = cell->card.value();
        
        switch (filter) {
            case AdjacentTargetSelector::FilterType::ALL:
                return true;
                
            case AdjacentTargetSelector::FilterType::ALLIES_ONLY:
                return card->getOwner() == owner;
                
            case AdjacentTargetSelector::FilterType::ENEMIES_ONLY:
                // Solo enemigos, sin contar líderes
                return card->getOwner() != owner && 
                       std::dynamic_pointer_cast<Legend>(card) == nullptr;
                
            case AdjacentTargetSelector::FilterType::NON_LEADERS:
                // Todas las cartas excepto líderes
                return std::dynamic_pointer_cast<Legend>(card) == nullptr;
        }
        
        return false;
    }
};

// Selector para efectos de estado de juego (no afecta cartas directamente)
class GameStateTargetSelector : public TargetSelector {
public:
    GameStateTargetSelector(CardPtr src, PlayerId ownerId)
        : TargetSelector(src, ownerId, "GameState") {}
    
    std::list<MapCell*> selectTargets(GameMap& gameMap, MapCell* triggerCell) override {
        (void)gameMap; // Suppress unused parameter warning
        (void)triggerCell; // Suppress unused parameter warning
        // Los efectos de estado de juego no seleccionan celdas específicas
        return std::list<MapCell*>();
    }
};
