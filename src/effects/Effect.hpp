#pragma once
#include "../utils/Types.hpp"
#include <string>
#include <memory>

// Forward declarations
class GameMap;
class MapCell;

// Clase base minimal para efectos - mantiene compatibilidad con el sistema legacy
// mientras migramos completamente al nuevo sistema basado en EffectImpl
class Effect {
protected:
    CardPtr source;
    PlayerId owner;
    std::string name;

public:
    Effect(CardPtr src, PlayerId ownerId, const std::string& effectName) 
        : source(src), owner(ownerId), name(effectName) {}
    
    virtual ~Effect() = default;
    
    // Métodos básicos que deben ser implementados por las clases derivadas
    virtual void execute(GameMap& gameMap, MapCell* triggerCell) = 0;
    virtual bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) = 0;
    virtual void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) = 0;
    virtual operator std::string() const = 0;
    
    // Getters
    CardPtr getSource() const { return source; }
    PlayerId getOwner() const { return owner; }
    const std::string& getName() const { return name; }
};

// Tipos para compatibilidad
using EffectPtr = std::shared_ptr<Effect>;
