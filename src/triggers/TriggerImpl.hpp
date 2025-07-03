#pragma once
#include "Trigger.hpp"
#include <array>

// Trigger que se activa inmediatamente al jugar la carta
class OnPlayTrigger : public Trigger {
public:
    OnPlayTrigger(CardPtr src, PlayerId ownerId)
        : Trigger(src, ownerId, "OnPlay") {}
    
    bool shouldActivate(GameMap& gameMap, MapCell* triggerCell) override {
        // Este trigger se activa una sola vez al jugar la carta
        // La lógica de activación se maneja externamente (en el GameState)
        (void)gameMap; (void)triggerCell;
        return true;
    }
};

// Trigger que se activa al lanzar un hechizo
class OnCastTrigger : public Trigger {
public:
    OnCastTrigger(CardPtr src, PlayerId ownerId)
        : Trigger(src, ownerId, "OnCast") {}
    
    bool shouldActivate(GameMap& gameMap, MapCell* triggerCell) override {
        // Este trigger se activa una sola vez al lanzar un hechizo
        // La lógica de activación se maneja externamente (en el GameState)
        (void)gameMap; (void)triggerCell;
        return true;
    }
};

// Trigger que se activa al inicio del turno
class OnStartTurnTrigger : public Trigger {
public:
    OnStartTurnTrigger(CardPtr src, PlayerId ownerId)
        : Trigger(src, ownerId, "OnStartTurn") {}
    
    bool shouldActivate(GameMap& gameMap, MapCell* triggerCell) override {
        // Este trigger se activa al inicio del turno
        // La lógica de activación se maneja externamente (en el GameState)
        (void)gameMap; (void)triggerCell;
        return true;
    }
};

// Trigger que se activa al final del turno
class OnEndTurnTrigger : public Trigger {
public:
    OnEndTurnTrigger(CardPtr src, PlayerId ownerId)
        : Trigger(src, ownerId, "OnEndTurn") {}
    
    bool shouldActivate(GameMap& gameMap, MapCell* triggerCell) override {
        // Este trigger se activa al final del turno
        // La lógica de activación se maneja externamente (en el GameState)
        (void)gameMap; (void)triggerCell;
        return true;
    }
};

// Trigger que se activa cuando una unidad enemiga entra en una dirección específica
class OnEnemyEnterTrigger : public Trigger {
private:
    std::vector<GameMap::Adjacency> watchDirections;
    std::array<bool, 6> appliedEffects; // Estado por dirección

public:
    // Constructor para una sola dirección
    OnEnemyEnterTrigger(CardPtr src, PlayerId ownerId, GameMap::Adjacency direction)
        : Trigger(src, ownerId, "OnEnemyEnter") {
        watchDirections.push_back(direction);
        appliedEffects.fill(false);
    }
    
    // Constructor para múltiples direcciones
    OnEnemyEnterTrigger(CardPtr src, PlayerId ownerId, const std::vector<GameMap::Adjacency>& directions)
        : Trigger(src, ownerId, "OnEnemyEnter"), watchDirections(directions) {
        appliedEffects.fill(false);
    }
    
    bool shouldActivate(GameMap& gameMap, MapCell* triggerCell) override {
        if (!triggerCell || watchDirections.empty()) return false;
        
        // Verificar que la carta fuente siga en el mapa
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return false;
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        MapCell* sourceCell = gameMap.at(sourceX, sourceY);
        if (!sourceCell) return false;
        
        // Verificar cada dirección monitoreada
        for (const auto& direction : watchDirections) {
            MapCell* watchedCell = gameMap.getNeighbor(direction, sourceCell, owner);
            if (!watchedCell) continue;
            
            // Índice de la dirección para el array de estado
            uint8_t directionIndex = static_cast<uint8_t>(direction);
            
            // Verificar si este es el trigger cell
            if (triggerCell == watchedCell) {
                // Activar si hay una carta enemiga y no se ha aplicado antes
                if (watchedCell->card.has_value() && 
                    watchedCell->card.value()->getOwner() != owner && 
                    !appliedEffects[directionIndex]) {
                    
                    appliedEffects[directionIndex] = true;
                    return true;
                }
            }
            
            // Limpiar estado si la celda está ahora vacía
            if (!watchedCell->card.has_value()) {
                appliedEffects[directionIndex] = false;
            }
        }
        
        return false;
    }
    
    // Operador para conversión a string (para depuración)
    operator std::string() const override {
        std::string result = "OnEnemyEnterTrigger [Directions: ";
        for (const auto& dir : watchDirections) {
            result += std::to_string(static_cast<int>(dir)) + " ";
        }
        result += "]";
        return result;
    }
};

// Trigger que se activa cuando una unidad enemiga sale de una dirección específica
class OnEnemyExitTrigger : public Trigger {
private:
    std::vector<GameMap::Adjacency> watchDirections;
    std::array<bool, 6> hadEnemyBefore; // Estado por dirección: tenía enemigo antes

public:
    // Constructor para una sola dirección
    OnEnemyExitTrigger(CardPtr src, PlayerId ownerId, GameMap::Adjacency direction)
        : Trigger(src, ownerId, "OnEnemyExit") {
        watchDirections.push_back(direction);
        hadEnemyBefore.fill(false);
    }
    
    // Constructor para múltiples direcciones
    OnEnemyExitTrigger(CardPtr src, PlayerId ownerId, const std::vector<GameMap::Adjacency>& directions)
        : Trigger(src, ownerId, "OnEnemyExit"), watchDirections(directions) {
        hadEnemyBefore.fill(false);
    }
    
    bool shouldActivate(GameMap& gameMap, MapCell* triggerCell) override {
        if (!triggerCell || watchDirections.empty()) return false;
        
        // Verificar que la carta fuente siga en el mapa
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return false;
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        MapCell* sourceCell = gameMap.at(sourceX, sourceY);
        if (!sourceCell) return false;
        
        // Verificar cada dirección monitoreada
        for (const auto& direction : watchDirections) {
            MapCell* watchedCell = gameMap.getNeighbor(direction, sourceCell, owner);
            if (!watchedCell) continue;
            
            uint8_t directionIndex = static_cast<uint8_t>(direction);
            
            // Si la celda observada está vacía y antes tenía un enemigo
            if ((!watchedCell->card.has_value() || 
                 watchedCell->card.value()->getOwner() == owner) && 
                hadEnemyBefore[directionIndex] && 
                triggerCell == watchedCell) {
                
                hadEnemyBefore[directionIndex] = false;
                return true;
            }
            
            // Actualizar estado: tiene enemigo ahora
            if (watchedCell->card.has_value() && watchedCell->card.value()->getOwner() != owner) {
                hadEnemyBefore[directionIndex] = true;
            }
        }
        
        return false;
    }
};

// Trigger que se activa cuando una unidad aliada entra en una dirección específica
class OnAllyEnterTrigger : public Trigger {
private:
    std::vector<GameMap::Adjacency> watchDirections;
    std::array<bool, 6> appliedEffects; // Estado por dirección

public:
    // Constructor para una sola dirección
    OnAllyEnterTrigger(CardPtr src, PlayerId ownerId, GameMap::Adjacency direction)
        : Trigger(src, ownerId, "OnAllyEnter") {
        watchDirections.push_back(direction);
        appliedEffects.fill(false);
    }
    
    // Constructor para múltiples direcciones
    OnAllyEnterTrigger(CardPtr src, PlayerId ownerId, const std::vector<GameMap::Adjacency>& directions)
        : Trigger(src, ownerId, "OnAllyEnter"), watchDirections(directions) {
        appliedEffects.fill(false);
    }
    
    bool shouldActivate(GameMap& gameMap, MapCell* triggerCell) override {
        if (!triggerCell || watchDirections.empty()) return false;
        
        // Verificar que la carta fuente siga en el mapa
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(source);
        if (!sourceUnit || !sourceUnit->isOnMap()) return false;
        
        auto [sourceX, sourceY] = sourceUnit->getCoordinates();
        MapCell* sourceCell = gameMap.at(sourceX, sourceY);
        if (!sourceCell) return false;
        
        // Verificar cada dirección monitoreada
        for (const auto& direction : watchDirections) {
            MapCell* watchedCell = gameMap.getNeighbor(direction, sourceCell, owner);
            if (!watchedCell) continue;
            
            uint8_t directionIndex = static_cast<uint8_t>(direction);
            
            // Verificar si este es el trigger cell
            if (triggerCell == watchedCell) {
                // Activar si hay una carta aliada y no se ha aplicado antes
                if (watchedCell->card.has_value() && 
                    watchedCell->card.value()->getOwner() == owner && 
                    !appliedEffects[directionIndex]) {
                    
                    appliedEffects[directionIndex] = true;
                    return true;
                }
            }
            
            // Limpiar estado si la celda está ahora vacía
            if (!watchedCell->card.has_value()) {
                appliedEffects[directionIndex] = false;
            }
        }
        
        return false;
    }
};
