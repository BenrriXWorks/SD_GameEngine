#pragma once
#include "../game/GameMap.hpp"
#include "../game/MapCell.hpp"
#include "../cards/Card.hpp"
#include <memory>
#include <string>
#include <vector>
#include <list>

// Forward declarations
class GameState;

// Clase base para seleccionar objetivos de efectos
// Esta clase define A QUIÉN/QUÉ afecta un efecto, sin preocuparse por QUÉ hace o CUÁNDO se activa
class TargetSelector {
protected:
    CardPtr source;
    PlayerId owner;
    std::string name;

public:
    TargetSelector(CardPtr src, PlayerId ownerId, const std::string& selectorName) 
        : source(src), owner(ownerId), name(selectorName) {}
    
    virtual ~TargetSelector() = default;
    
    // Método principal que selecciona los objetivos
    virtual std::list<MapCell*> selectTargets(GameMap& gameMap, MapCell* triggerCell) = 0;
    
    // Método para seleccionar objetivos de cartas (para efectos que afectan cartas específicas)
    virtual std::list<CardPtr> selectCardTargets(GameMap& gameMap, MapCell* triggerCell) {
        std::list<CardPtr> cards;
        auto cells = selectTargets(gameMap, triggerCell);
        for (auto cell : cells) {
            if (cell && cell->card.has_value()) {
                cards.push_back(cell->card.value());
            }
        }
        return cards;
    }
    
    // Método para verificar si un objetivo es válido
    virtual bool isValidTarget(MapCell* cell) const {
        return cell != nullptr;
    }
    
    // Getter para el nombre del selector
    const std::string& getName() const { return name; }
    
    // Operador para conversión a string (para depuración)
    virtual operator std::string() const {
        return "TargetSelector: " + name;
    }
};

using TargetSelectorPtr = std::shared_ptr<TargetSelector>;
