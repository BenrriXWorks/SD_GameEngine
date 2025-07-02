#pragma once
#include "../Effect.hpp"
#include "../../cards/Card.hpp"
#include "../../game/GameMap.hpp"
#include <functional>
#include <vector>

// Efecto base para selección de objetivos
class SelectionEffect : public Effect {
public:
    enum class SelectionType {
        SELECT_ALLY,    // Seleccionar una unidad aliada
        SELECT_ENEMY,   // Seleccionar una unidad enemiga (no leyenda)
        SELECT_ANY      // Seleccionar cualquier unidad
    };

protected:
    SelectionType selectionType;
    std::function<void(std::shared_ptr<Unit>)> effectFunction;
    uint8_t targetX, targetY; // Posición del objetivo seleccionado
    bool hasTarget;

public:
    SelectionEffect(std::shared_ptr<Card> source, PlayerId owner, const std::string& name,
                   SelectionType type, std::function<void(std::shared_ptr<Unit>)> func)
        : Effect(source, owner, name), selectionType(type), effectFunction(func), 
          targetX(255), targetY(255), hasTarget(false) {}

    // Establecer el objetivo seleccionado
    void setTarget(uint8_t x, uint8_t y) {
        targetX = x;
        targetY = y;
        hasTarget = true;
    }

    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)triggerCell; (void)targetCells;
        
        if (!hasTarget) return false;
        
        MapCell* targetCell = gameMap.at(targetX, targetY);
        if (!targetCell || !targetCell->card.has_value()) return false;
        
        auto targetUnit = std::dynamic_pointer_cast<Unit>(targetCell->card.value());
        if (!targetUnit) return false;
        
        // Verificar que el objetivo es válido según el tipo de selección
        return isValidTarget(targetUnit);
    }

    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)map; (void)triggerCell; (void)affectedCells;
        
        if (!hasTarget) return;
        
        MapCell* targetCell = map.at(targetX, targetY);
        if (!targetCell || !targetCell->card.has_value()) return;
        
        auto targetUnit = std::dynamic_pointer_cast<Unit>(targetCell->card.value());
        if (!targetUnit || !isValidTarget(targetUnit)) return;
        
        // Aplicar el efecto al objetivo
        effectFunction(targetUnit);
    }

private:
    bool isValidTarget(std::shared_ptr<Unit> target) {
        switch (selectionType) {
            case SelectionType::SELECT_ALLY:
                return target->getOwner() == owner;
                
            case SelectionType::SELECT_ENEMY:
                // No puede seleccionar enemigos que sean leyendas
                return target->getOwner() != owner && !target->isLegend();
                
            case SelectionType::SELECT_ANY:
                return true;
        }
        return false;
    }
};

// Efecto de curación dirigida (seleccionar aliado)
class TargetedHealEffect : public SelectionEffect {
private:
    uint8_t healAmount;

public:
    TargetedHealEffect(std::shared_ptr<Card> source, PlayerId owner, uint8_t healing)
        : SelectionEffect(source, owner, "TargetedHeal", SelectionType::SELECT_ALLY,
                         [healing](std::shared_ptr<Unit> unit) {
                             unit->heal(healing);
                         }),
          healAmount(healing) {}
};

// Efecto de daño dirigido (seleccionar enemigo, no leyenda)
class TargetedDamageEffect : public SelectionEffect {
private:
    uint8_t damage;

public:
    TargetedDamageEffect(std::shared_ptr<Card> source, PlayerId owner, uint8_t dmg)
        : SelectionEffect(source, owner, "TargetedDamage", SelectionType::SELECT_ENEMY,
                         [dmg](std::shared_ptr<Unit> unit) {
                             unit->takeDamage(dmg);
                         }),
          damage(dmg) {}
};

// Efecto de buff dirigido (seleccionar aliado)
class TargetedBuffEffect : public SelectionEffect {
private:
    int attackBuff;
    int healthBuff;

public:
    TargetedBuffEffect(std::shared_ptr<Card> source, PlayerId owner, int atkBuff, int hpBuff)
        : SelectionEffect(source, owner, "TargetedBuff", SelectionType::SELECT_ALLY,
                         [atkBuff, hpBuff](std::shared_ptr<Unit> unit) {
                             if (atkBuff != 0) {
                                 int newAttack = static_cast<int>(unit->getAttack()) + atkBuff;
                                 unit->setAttack(std::max(0, newAttack));
                             }
                             if (hpBuff > 0) {
                                 unit->setMaxHealth(unit->getMaxHealth() + hpBuff);
                                 unit->heal(hpBuff);
                             }
                         }),
          attackBuff(atkBuff), healthBuff(hpBuff) {}
};

// Efecto de debuff dirigido (seleccionar enemigo, no leyenda)
class TargetedDebuffEffect : public SelectionEffect {
private:
    int attackDebuff;
    int speedDebuff;

public:
    TargetedDebuffEffect(std::shared_ptr<Card> source, PlayerId owner, int atkDebuff, int spdDebuff)
        : SelectionEffect(source, owner, "TargetedDebuff", SelectionType::SELECT_ENEMY,
                         [atkDebuff, spdDebuff](std::shared_ptr<Unit> unit) {
                             if (atkDebuff != 0) {
                                 int newAttack = static_cast<int>(unit->getAttack()) + atkDebuff;
                                 unit->setAttack(std::max(0, newAttack));
                             }
                             if (spdDebuff != 0) {
                                 int newSpeed = static_cast<int>(unit->getSpeed()) + spdDebuff;
                                 unit->setSpeed(std::max(1, newSpeed)); // Mínimo speed = 1
                             }
                         }),
          attackDebuff(atkDebuff), speedDebuff(spdDebuff) {}
};
