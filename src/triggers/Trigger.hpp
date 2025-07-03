#pragma once
#include "../game/GameMap.hpp"
#include "../game/MapCell.hpp"
#include "../cards/Card.hpp"
#include <memory>
#include <string>
#include <vector>

// Forward declarations
class GameState;

// Clase base para todos los triggers
class Trigger {
protected:
    CardPtr source;
    PlayerId owner;
    std::string name;

public:
    Trigger(CardPtr src, PlayerId ownerId, const std::string& triggerName) 
        : source(src), owner(ownerId), name(triggerName) {}
    
    virtual ~Trigger() = default;
    
    // Método principal que verifica si el trigger debe activarse
    virtual bool shouldActivate(GameMap& gameMap, MapCell* triggerCell) = 0;
    
    // Getter para el nombre del trigger
    const std::string& getName() const { return name; }
    
    // Operador para conversión a string (para depuración)
    virtual operator std::string() const {
        return "Trigger: " + name;
    }
};

using TriggerPtr = std::shared_ptr<Trigger>;
