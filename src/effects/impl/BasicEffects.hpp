#pragma once
#include "../Effect.hpp"
#include "../../cards/Card.hpp"
#include "../../game/GameMap.hpp"
#include <algorithm>
#include <functional>

// Basic attack modifier effect
class AttackModifierEffect : public Effect {
private:
    int attackChange;
    
public:
    enum class TargetType {
        SELF,           // Only affects the source card
        ADJACENT,       // Affects adjacent cards
        ALL_FRIENDLY,   // Affects all friendly cards
        ALL_ENEMY,      // Affects all enemy cards
        SPECIFIC_POSITION // Affects card at specific position (for spells)
    };
    
    TargetType targetType;
    
    AttackModifierEffect(std::shared_ptr<Card> source, PlayerId owner, int attackChange, TargetType targetType)
        : Effect(source, owner, "AttackModifier"), attackChange(attackChange), targetType(targetType) {}
    
    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)gameMap; (void)triggerCell; (void)targetCells;
        return true; // Basic effects always apply
    }
    
    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)map; (void)triggerCell; (void)affectedCells;
        auto sourceCard = source;
        if (!sourceCard) return;
        
        switch (targetType) {
            case TargetType::SELF: {
                if (auto unit = std::dynamic_pointer_cast<Unit>(sourceCard)) {
                    unit->setAttack(unit->getAttack() + attackChange);
                }
                break;
            }
            case TargetType::ADJACENT: {
                applyToAdjacent(map, sourceCard, [this](std::shared_ptr<Unit> unit) {
                    unit->setAttack(unit->getAttack() + attackChange);
                });
                break;
            }
            case TargetType::ALL_FRIENDLY: {
                applyToAllOfType(map, true, [this](std::shared_ptr<Unit> unit) {
                    unit->setAttack(unit->getAttack() + attackChange);
                });
                break;
            }
            case TargetType::ALL_ENEMY: {
                applyToAllOfType(map, false, [this](std::shared_ptr<Unit> unit) {
                    unit->setAttack(unit->getAttack() + attackChange);
                });
                break;
            }
            case TargetType::SPECIFIC_POSITION: {
                if (triggerCell && triggerCell->card.has_value()) {
                    if (auto unit = std::dynamic_pointer_cast<Unit>(triggerCell->card.value())) {
                        unit->setAttack(unit->getAttack() + attackChange);
                    }
                }
                break;
            }
        }
    }
    
private:
    void applyToAdjacent(GameMap& map, std::shared_ptr<Card> sourceCard, std::function<void(std::shared_ptr<Unit>)> modifier) {
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(sourceCard);
        if (!sourceUnit || !sourceUnit->isOnMap()) return;
        
        auto [x, y] = sourceUnit->getCoordinates();
        
        for (auto adj : {GameMap::Adjacency::TOP_LEFT, GameMap::Adjacency::TOP_RIGHT,
                        GameMap::Adjacency::BOTTOM_LEFT, GameMap::Adjacency::BOTTOM_RIGHT,
                        GameMap::Adjacency::UP, GameMap::Adjacency::DOWN}) {
            if (auto cell = map.getNeighbor(adj, map.at(x, y), owner)) {
                if (cell->card.has_value()) {
                    if (auto unit = std::dynamic_pointer_cast<Unit>(cell->card.value())) {
                        // Only affect friendly units for buffs, enemies for debuffs
                        bool isFriendly = (unit->getOwner() == owner);
                        if ((attackChange > 0 && isFriendly) || (attackChange < 0 && !isFriendly)) {
                            modifier(unit);
                        }
                    }
                }
            }
        }
    }
    
    void applyToAllOfType(GameMap& map, bool friendly, std::function<void(std::shared_ptr<Unit>)> modifier) {
        for (uint8_t x = 0; x < GameMap::MAP_WIDTH; ++x) {
            for (uint8_t y = 0; y < GameMap::MAP_HEIGHT; ++y) {
                if (auto cell = map.at(x, y)) {
                    if (cell->card.has_value()) {
                        if (auto unit = std::dynamic_pointer_cast<Unit>(cell->card.value())) {
                            bool isFriendly = (unit->getOwner() == owner);
                            if (isFriendly == friendly) {
                                modifier(unit);
                            }
                        }
                    }
                }
            }
        }
    }
};

// Basic health modifier effect
class HealthModifierEffect : public Effect {
private:
    int healthChange;
    AttackModifierEffect::TargetType targetType;
    
public:
    HealthModifierEffect(std::shared_ptr<Card> source, PlayerId owner, int healthChange, AttackModifierEffect::TargetType targetType)
        : Effect(source, owner, "HealthModifier"), healthChange(healthChange), targetType(targetType) {}
    
    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)gameMap; (void)triggerCell; (void)targetCells;
        return true; // Basic effects always apply
    }
    
    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)map; (void)triggerCell; (void)affectedCells;
        auto sourceCard = source;
        if (!sourceCard) return;
        
        switch (targetType) {
            case AttackModifierEffect::TargetType::SELF: {
                if (auto unit = std::dynamic_pointer_cast<Unit>(sourceCard)) {
                    if (healthChange > 0) {
                        unit->heal(healthChange);
                    } else {
                        unit->takeDamage(-healthChange);
                    }
                }
                break;
            }
            case AttackModifierEffect::TargetType::ADJACENT: {
                applyToAdjacent(map, sourceCard, [this](std::shared_ptr<Unit> unit) {
                    if (healthChange > 0) {
                        unit->heal(healthChange);
                    } else {
                        unit->takeDamage(-healthChange);
                    }
                });
                break;
            }
            case AttackModifierEffect::TargetType::ALL_FRIENDLY: {
                applyToAllOfType(map, true, [this](std::shared_ptr<Unit> unit) {
                    if (healthChange > 0) {
                        unit->heal(healthChange);
                    } else {
                        unit->takeDamage(-healthChange);
                    }
                });
                break;
            }
            case AttackModifierEffect::TargetType::ALL_ENEMY: {
                applyToAllOfType(map, false, [this](std::shared_ptr<Unit> unit) {
                    if (healthChange > 0) {
                        unit->heal(healthChange);
                    } else {
                        unit->takeDamage(-healthChange);
                    }
                });
                break;
            }
            case AttackModifierEffect::TargetType::SPECIFIC_POSITION: {
                if (triggerCell && triggerCell->card.has_value()) {
                    if (auto unit = std::dynamic_pointer_cast<Unit>(triggerCell->card.value())) {
                        if (healthChange > 0) {
                            unit->heal(healthChange);
                        } else {
                            unit->takeDamage(-healthChange);
                        }
                    }
                }
                break;
            }
        }
    }
    
private:
    void applyToAdjacent(GameMap& map, std::shared_ptr<Card> sourceCard, std::function<void(std::shared_ptr<Unit>)> modifier) {
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(sourceCard);
        if (!sourceUnit || !sourceUnit->isOnMap()) return;
        
        auto [x, y] = sourceUnit->getCoordinates();
        
        for (auto adj : {GameMap::Adjacency::TOP_LEFT, GameMap::Adjacency::TOP_RIGHT,
                        GameMap::Adjacency::BOTTOM_LEFT, GameMap::Adjacency::BOTTOM_RIGHT,
                        GameMap::Adjacency::UP, GameMap::Adjacency::DOWN}) {
            if (auto cell = map.getNeighbor(adj, map.at(x, y), owner)) {
                if (cell->card.has_value()) {
                    if (auto unit = std::dynamic_pointer_cast<Unit>(cell->card.value())) {
                        bool isFriendly = (unit->getOwner() == owner);
                        if ((healthChange > 0 && isFriendly) || (healthChange < 0 && !isFriendly)) {
                            modifier(unit);
                        }
                    }
                }
            }
        }
    }
    
    void applyToAllOfType(GameMap& map, bool friendly, std::function<void(std::shared_ptr<Unit>)> modifier) {
        for (uint8_t x = 0; x < GameMap::MAP_WIDTH; ++x) {
            for (uint8_t y = 0; y < GameMap::MAP_HEIGHT; ++y) {
                if (auto cell = map.at(x, y)) {
                    if (cell->card.has_value()) {
                        if (auto unit = std::dynamic_pointer_cast<Unit>(cell->card.value())) {
                            bool isFriendly = (unit->getOwner() == owner);
                            if (isFriendly == friendly) {
                                modifier(unit);
                            }
                        }
                    }
                }
            }
        }
    }
};

// Basic range modifier effect
class RangeModifierEffect : public Effect {
private:
    int rangeChange;
    AttackModifierEffect::TargetType targetType;
    
public:
    RangeModifierEffect(std::shared_ptr<Card> source, PlayerId owner, int rangeChange, AttackModifierEffect::TargetType targetType)
        : Effect(source, owner, "RangeModifier"), rangeChange(rangeChange), targetType(targetType) {}
    
    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)gameMap; (void)triggerCell; (void)targetCells;
        return true; // Basic effects always apply
    }
    
    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)map; (void)triggerCell; (void)affectedCells;
        auto sourceCard = source;
        if (!sourceCard) return;
        
        switch (targetType) {
            case AttackModifierEffect::TargetType::SELF: {
                if (auto unit = std::dynamic_pointer_cast<Unit>(sourceCard)) {
                    unit->setRange(std::max(1, (int)unit->getRange() + rangeChange));
                }
                break;
            }
            case AttackModifierEffect::TargetType::ADJACENT: {
                applyToAdjacent(map, sourceCard, [this](std::shared_ptr<Unit> unit) {
                    unit->setRange(std::max(1, (int)unit->getRange() + rangeChange));
                });
                break;
            }
            case AttackModifierEffect::TargetType::ALL_FRIENDLY: {
                applyToAllOfType(map, true, [this](std::shared_ptr<Unit> unit) {
                    unit->setRange(std::max(1, (int)unit->getRange() + rangeChange));
                });
                break;
            }
            case AttackModifierEffect::TargetType::ALL_ENEMY: {
                applyToAllOfType(map, false, [this](std::shared_ptr<Unit> unit) {
                    unit->setRange(std::max(1, (int)unit->getRange() + rangeChange));
                });
                break;
            }
            case AttackModifierEffect::TargetType::SPECIFIC_POSITION: {
                if (triggerCell && triggerCell->card.has_value()) {
                    if (auto unit = std::dynamic_pointer_cast<Unit>(triggerCell->card.value())) {
                        unit->setRange(std::max(1, (int)unit->getRange() + rangeChange));
                    }
                }
                break;
            }
        }
    }
    
private:
    void applyToAdjacent(GameMap& map, std::shared_ptr<Card> sourceCard, std::function<void(std::shared_ptr<Unit>)> modifier) {
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(sourceCard);
        if (!sourceUnit || !sourceUnit->isOnMap()) return;
        
        auto [x, y] = sourceUnit->getCoordinates();
        
        for (auto adj : {GameMap::Adjacency::TOP_LEFT, GameMap::Adjacency::TOP_RIGHT,
                        GameMap::Adjacency::BOTTOM_LEFT, GameMap::Adjacency::BOTTOM_RIGHT,
                        GameMap::Adjacency::UP, GameMap::Adjacency::DOWN}) {
            if (auto cell = map.getNeighbor(adj, map.at(x, y), owner)) {
                if (cell->card.has_value()) {
                    if (auto unit = std::dynamic_pointer_cast<Unit>(cell->card.value())) {
                        bool isFriendly = (unit->getOwner() == owner);
                        if ((rangeChange > 0 && isFriendly) || (rangeChange < 0 && !isFriendly)) {
                            modifier(unit);
                        }
                    }
                }
            }
        }
    }
    
    void applyToAllOfType(GameMap& map, bool friendly, std::function<void(std::shared_ptr<Unit>)> modifier) {
        for (uint8_t x = 0; x < GameMap::MAP_WIDTH; ++x) {
            for (uint8_t y = 0; y < GameMap::MAP_HEIGHT; ++y) {
                if (auto cell = map.at(x, y)) {
                    if (cell->card.has_value()) {
                        if (auto unit = std::dynamic_pointer_cast<Unit>(cell->card.value())) {
                            bool isFriendly = (unit->getOwner() == owner);
                            if (isFriendly == friendly) {
                                modifier(unit);
                            }
                        }
                    }
                }
            }
        }
    }
};

// Positional trigger effect - triggers when enemy enters specific position relative to source
class PositionalTriggerEffect : public Effect {
private:
    GameMap::Adjacency triggerDirection;
    std::shared_ptr<Effect> triggeredEffect;
    
public:
    PositionalTriggerEffect(std::shared_ptr<Card> source, PlayerId owner, GameMap::Adjacency direction, std::shared_ptr<Effect> effect)
        : Effect(source, owner, "PositionalTrigger"), triggerDirection(direction), triggeredEffect(effect) {}
    
    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)gameMap; (void)triggerCell; (void)targetCells;
        return true; // Trigger effects always check
    }
    
    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)map; (void)triggerCell; (void)affectedCells;
        auto sourceCard = source;
        auto sourceUnit = std::dynamic_pointer_cast<Unit>(sourceCard);
        if (!sourceUnit || !sourceUnit->isOnMap()) return;
        
        // Check if trigger condition is met
        if (triggerCell && triggerCell->card.has_value()) {
            auto triggerUnit = std::dynamic_pointer_cast<Unit>(triggerCell->card.value());
            if (triggerUnit && triggerUnit->getOwner() != owner) {
                // Check if the trigger cell is in the correct direction from source
                auto [sourceX, sourceY] = sourceUnit->getCoordinates();
                auto sourceCell = map.at(sourceX, sourceY);
                auto expectedCell = map.getNeighbor(triggerDirection, sourceCell, owner);
                
                if (expectedCell == triggerCell) {
                    // Trigger condition met - apply the triggered effect
                    if (triggeredEffect) {
                        triggeredEffect->apply(map, triggerCell, affectedCells);
                    }
                }
            }
        }
    }
};
