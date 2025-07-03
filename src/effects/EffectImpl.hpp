#pragma once
#include "../game/GameMap.hpp"
#include "../game/MapCell.hpp"
#include "../cards/Card.hpp"
#include "../targeting/TargetSelector.hpp"
#include <list>
#include <memory>
#include <string>

// Forward declarations
class GameState;

// Clase base para las implementaciones de efectos
// Esta clase define QUÉ hace un efecto, sin preocuparse por CUÁNDO se activa o A QUIÉN afecta
class EffectImpl {
protected:
    CardPtr source;
    PlayerId owner;
    std::string name;

public:
    EffectImpl(CardPtr src, PlayerId ownerId, const std::string& effectName) 
        : source(src), owner(ownerId), name(effectName) {}
    
    virtual ~EffectImpl() = default;
    
    // Método principal que implementa la lógica del efecto
    // Los targets ya están seleccionados por el TargetSelector
    virtual void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) = 0;
    
    // Método alternativo para efectos que afectan cartas directamente
    virtual void applyToCards(GameMap& gameMap, MapCell* triggerCell, const std::list<CardPtr>& targetCards) {
        // Por defecto, convertir cartas a celdas y usar apply normal
        std::list<MapCell*> targetCells;
        for (auto card : targetCards) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                if (unit->isOnMap()) {
                    auto [x, y] = unit->getCoordinates();
                    MapCell* cell = gameMap.at(x, y);
                    if (cell) {
                        targetCells.push_back(cell);
                    }
                }
            }
        }
        apply(gameMap, triggerCell, targetCells);
    }
    
    // Método para efectos de estado de juego (como robar cartas)
    virtual void applyToGameState(GameMap& gameMap, MapCell* triggerCell, GameState* gameState) {
        // Por defecto, no hacer nada - solo efectos específicos lo implementarán
        (void)gameMap; (void)triggerCell; (void)gameState;
    }
    
    // Método para verificar si el efecto es aplicable (condiciones adicionales)
    virtual bool isApplicable(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) {
        (void)gameMap; (void)triggerCell; (void)targetCells; // Suprimir warnings de parámetros no usados
        
        // Por defecto, verificar que la carta fuente existe y está viva (para unidades)
        if (!source) return false;
        
        // Para cartas de tipo Unit, verificar que están vivas
        if (auto unit = std::dynamic_pointer_cast<Unit>(source)) {
            if (!unit->isAlive()) return false;
        }
        
        return true;
    }
    
    // Getter para el nombre del efecto
    const std::string& getName() const { return name; }
    
    // Operador para conversión a string (para depuración)
    virtual operator std::string() const {
        return "EffectImpl: " + name;
    }
};

using EffectImplPtr = std::shared_ptr<EffectImpl>;
