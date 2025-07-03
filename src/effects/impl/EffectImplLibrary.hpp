#pragma once
#include "../EffectImpl.hpp"
#include "../../cards/Card.hpp"
#include "../../game/GameState.hpp"
#include <print>

// ================================
// EFECTOS DE MODIFICACIÓN DE CARTAS
// ================================

// Efecto que modifica el ataque de cartas
class AttackModifierEffectImpl : public EffectImpl {
private:
    int attackChange;

public:
    AttackModifierEffectImpl(CardPtr src, PlayerId ownerId, int change)
        : EffectImpl(src, ownerId, change > 0 ? "AttackBuff" : "AttackDebuff"), 
          attackChange(change) {}
    
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto cell : targetCells) {
            if (cell && cell->card.has_value()) {
                auto card = cell->card.value();
                if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                    int newAttack = std::max(0, static_cast<int>(unit->getAttack()) + attackChange);
                    unit->setAttack(newAttack);
                }
            }
        }
    }
    
    void applyToCards(GameMap& gameMap, MapCell* triggerCell, const std::list<CardPtr>& targetCards) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto card : targetCards) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                int newAttack = std::max(0, static_cast<int>(unit->getAttack()) + attackChange);
                unit->setAttack(newAttack);
            }
        }
    }
};

// Efecto que modifica la salud de cartas
class HealthModifierEffectImpl : public EffectImpl {
private:
    int healthChange;

public:
    HealthModifierEffectImpl(CardPtr src, PlayerId ownerId, int change)
        : EffectImpl(src, ownerId, change > 0 ? "HealthBuff" : "HealthDebuff"), 
          healthChange(change) {}
    
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto cell : targetCells) {
            if (cell && cell->card.has_value()) {
                auto card = cell->card.value();
                if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                    if (healthChange > 0) {
                        unit->heal(healthChange);
                    } else {
                        unit->takeDamage(-healthChange);
                    }
                }
            }
        }
    }
    
    void applyToCards(GameMap& gameMap, MapCell* triggerCell, const std::list<CardPtr>& targetCards) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto card : targetCards) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                if (healthChange > 0) {
                    unit->heal(healthChange);
                } else {
                    unit->takeDamage(-healthChange);
                }
            }
        }
    }
};

// Efecto que hace daño directo
class DamageEffectImpl : public EffectImpl {
private:
    uint8_t damageAmount;

public:
    DamageEffectImpl(CardPtr src, PlayerId ownerId, uint8_t damage)
        : EffectImpl(src, ownerId, "Damage"), damageAmount(damage) {}
    
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto cell : targetCells) {
            if (cell && cell->card.has_value()) {
                auto card = cell->card.value();
                if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                    unit->takeDamage(damageAmount);
                }
            }
        }
    }
    
    void applyToCards(GameMap& gameMap, MapCell* triggerCell, const std::list<CardPtr>& targetCards) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto card : targetCards) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                unit->takeDamage(damageAmount);
            }
        }
    }
};

// Efecto que cura cartas
class HealEffectImpl : public EffectImpl {
private:
    uint8_t healAmount;

public:
    HealEffectImpl(CardPtr src, PlayerId ownerId, uint8_t heal)
        : EffectImpl(src, ownerId, "Heal"), healAmount(heal) {}
    
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto cell : targetCells) {
            if (cell && cell->card.has_value()) {
                auto card = cell->card.value();
                if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                    unit->heal(healAmount);
                }
            }
        }
    }
    
    void applyToCards(GameMap& gameMap, MapCell* triggerCell, const std::list<CardPtr>& targetCards) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto card : targetCards) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                unit->heal(healAmount);
            }
        }
    }
};

// Efecto que modifica el alcance de cartas
class RangeModifierEffectImpl : public EffectImpl {
private:
    int rangeChange;

public:
    RangeModifierEffectImpl(CardPtr src, PlayerId ownerId, int change)
        : EffectImpl(src, ownerId, change > 0 ? "RangeBuff" : "RangeDebuff"), 
          rangeChange(change) {}
    
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto cell : targetCells) {
            if (cell && cell->card.has_value()) {
                auto card = cell->card.value();
                if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                    int newRange = std::max(1, static_cast<int>(unit->getRange()) + rangeChange);
                    unit->setRange(newRange);
                }
            }
        }
    }
    
    void applyToCards(GameMap& gameMap, MapCell* triggerCell, const std::list<CardPtr>& targetCards) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto card : targetCards) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                int newRange = std::max(1, static_cast<int>(unit->getRange()) + rangeChange);
                unit->setRange(newRange);
            }
        }
    }
};

// Efecto que modifica la velocidad de cartas
class SpeedModifierEffectImpl : public EffectImpl {
private:
    int speedChange;

public:
    SpeedModifierEffectImpl(CardPtr src, PlayerId ownerId, int change)
        : EffectImpl(src, ownerId, change > 0 ? "SpeedBuff" : "SpeedDebuff"), 
          speedChange(change) {}
    
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto cell : targetCells) {
            if (cell && cell->card.has_value()) {
                auto card = cell->card.value();
                if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                    int newSpeed = std::max(1, static_cast<int>(unit->getSpeed()) + speedChange);
                    unit->setSpeed(newSpeed);
                }
            }
        }
    }
    
    void applyToCards(GameMap& gameMap, MapCell* triggerCell, const std::list<CardPtr>& targetCards) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto card : targetCards) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                int newSpeed = std::max(1, static_cast<int>(unit->getSpeed()) + speedChange);
                unit->setSpeed(newSpeed);
            }
        }
    }
};

// Efecto que modifica el costo de cartas
class CostModifierEffectImpl : public EffectImpl {
private:
    int costChange;

public:
    CostModifierEffectImpl(CardPtr src, PlayerId ownerId, int change)
        : EffectImpl(src, ownerId, change > 0 ? "CostIncrease" : "CostReduction"), 
          costChange(change) {}
    
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        // Este efecto normalmente se aplicaría a cartas en la mano, no en el mapa
        // Pero por completitud, permitimos que funcione
        (void)gameMap; (void)triggerCell; (void)targetCells;
    }
    
    void applyToCards(GameMap& gameMap, MapCell* triggerCell, const std::list<CardPtr>& targetCards) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto card : targetCards) {
            int newCost = std::max(0, static_cast<int>(card->getCost()) + costChange);
            card->setCost(newCost);
        }
    }
};

// ================================
// EFECTOS DE ESTADO DE JUEGO
// ================================

// Efecto que roba cartas del deck
class DrawCardsEffectImpl : public EffectImpl {
private:
    uint8_t cardCount;

public:
    DrawCardsEffectImpl(CardPtr src, PlayerId ownerId, uint8_t count)
        : EffectImpl(src, ownerId, "DrawCards"), cardCount(count) {}
    
    // Este efecto no afecta celdas específicas
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        // No hace nada - necesita acceso al GameState
        (void)gameMap; (void)triggerCell; (void)targetCells;
    }
    
    void applyToGameState(GameMap& gameMap, MapCell* triggerCell, GameState* gameState) override {
        // DrawCard functionality disabled - no access to private drawCard method
        (void)gameMap; (void)triggerCell; (void)gameState;
    }
};

// Efecto que agrega maná
class AddManaEffectImpl : public EffectImpl {
private:
    uint8_t manaAmount;

public:
    AddManaEffectImpl(CardPtr src, PlayerId ownerId, uint8_t mana)
        : EffectImpl(src, ownerId, "AddMana"), manaAmount(mana) {}
    
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        // No hace nada - necesita acceso al GameState
        (void)gameMap; (void)triggerCell; (void)targetCells;
    }
    
    void applyToGameState(GameMap& gameMap, MapCell* triggerCell, GameState* gameState) override {
        // Sistema no usa mana, solo acciones - efecto deshabilitado
        (void)gameMap; (void)triggerCell; (void)gameState;
    }
};

// Efecto que reduce el costo de cartas en la mano
class ReduceHandCostEffectImpl : public EffectImpl {
private:
    uint8_t costReduction;

public:
    ReduceHandCostEffectImpl(CardPtr src, PlayerId ownerId, uint8_t reduction)
        : EffectImpl(src, ownerId, "ReduceHandCost"), costReduction(reduction) {}
    
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        // No hace nada - necesita acceso al GameState
        (void)gameMap; (void)triggerCell; (void)targetCells;
    }
    
    void applyToGameState(GameMap& gameMap, MapCell* triggerCell, GameState* gameState) override {
        // Sistema no maneja costos de mano de esta manera - efecto deshabilitado
        (void)gameMap; (void)triggerCell; (void)gameState;
    }
};

// ================================
// EFECTOS ESPECIALES
// ================================

// Efecto simple para imprimir mensajes de depuración
class PrintEffectImpl : public EffectImpl {
private:
    std::string message;

public:
    PrintEffectImpl(CardPtr src, PlayerId ownerId, const std::string& msg) 
        : EffectImpl(src, ownerId, "Print"), message(msg) {}

    // Solo implementa QUÉ hacer: imprimir un mensaje
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        (void)gameMap; (void)triggerCell; (void)targetCells;
        std::println("[DEBUG] {}: {}", source ? source->getName() : "Unknown", message);
    }
    
    void applyToCards(GameMap& gameMap, MapCell* triggerCell, const std::list<CardPtr>& targetCards) override {
        (void)gameMap; (void)triggerCell; (void)targetCards;
        std::println("[DEBUG] {}: {}", source ? source->getName() : "Unknown", message);
    }
    
    void applyToGameState(GameMap& gameMap, MapCell* triggerCell, GameState* gameState) override {
        (void)gameMap; (void)triggerCell; (void)gameState;
        std::println("[DEBUG] {}: {}", source ? source->getName() : "Unknown", message);
    }
};

// Efecto que destruye/elimina cartas
class DestroyEffectImpl : public EffectImpl {
public:
    DestroyEffectImpl(CardPtr src, PlayerId ownerId)
        : EffectImpl(src, ownerId, "Destroy") {}
    
    void apply(GameMap& gameMap, MapCell* triggerCell, const std::list<MapCell*>& targetCells) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto cell : targetCells) {
            if (cell && cell->card.has_value()) {
                auto card = cell->card.value();
                if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                    // Marcar como muerto (el GameState se encargará de la limpieza)
                    unit->takeDamage(unit->getHealth());
                }
            }
        }
    }
    
    void applyToCards(GameMap& gameMap, MapCell* triggerCell, const std::list<CardPtr>& targetCards) override {
        (void)gameMap; (void)triggerCell; // Suprimir warnings de parámetros no usados
        for (auto card : targetCards) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
                // Marcar como muerto
                unit->takeDamage(unit->getHealth());
            }
        }
    }
};
