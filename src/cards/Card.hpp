#pragma once
#include <string>
#include <list>
#include <memory>
#include <cstdint>
#include "../utils/Types.hpp"

// Forward declaration
class MapCell;

class Card {
protected:
    uint8_t id;
    std::string name;
    uint8_t cost;
    std::string description;
    PlayerId owner;
    std::string imageLink;
    std::list<EffectPtr> effects;

public:
    Card(uint8_t cardId, const std::string& cardName, uint8_t cardCost, 
         const std::string& desc, PlayerId ownerId, const std::string& image = "")
        : id(cardId), name(cardName), cost(cardCost), description(desc), 
          owner(ownerId), imageLink(image) {}

    virtual ~Card() = default;

    // Getters
    uint8_t getId() const { return id; }
    const std::string& getName() const { return name; }
    uint8_t getCost() const { return cost; }
    const std::string& getDescription() const { return description; }
    PlayerId getOwner() const { return owner; }
    const std::string& getImageLink() const { return imageLink; }
    const std::list<EffectPtr>& getEffects() const { return effects; }

    // Setters
    void setOwner(PlayerId newOwner) { owner = newOwner; }
    void addEffect(EffectPtr effect) { effects.push_back(effect); }
    void removeEffect(EffectPtr effect) { effects.remove(effect); }

    // Conversion to uint8_t for compatibility
    operator uint8_t() const { return id; }

    // Virtual format method
    virtual std::string format() const = 0;
    
    // String conversion
    operator std::string() const { return format(); }
};

class Unit : public Card {
protected:
    uint8_t attack;
    uint8_t health;
    uint8_t maxHealth;
    uint8_t speed;
    uint8_t range;
    
    // Position tracking - (255,255) if not on map
    uint8_t x = 255, y = 255;

public:
    Unit(uint8_t cardId, const std::string& cardName, uint8_t cardCost, 
         const std::string& desc, PlayerId ownerId, uint8_t atk, uint8_t hp, 
         uint8_t spd, uint8_t rng, const std::string& image = "")
        : Card(cardId, cardName, cardCost, desc, ownerId, image), 
          attack(atk), health(hp), maxHealth(hp), speed(spd), range(rng) {}

    // Getters
    uint8_t getAttack() const { return attack; }
    uint8_t getHealth() const { return health; }
    uint8_t getMaxHealth() const { return maxHealth; }
    uint8_t getSpeed() const { return speed; }
    uint8_t getRange() const { return range; }

    // Setters
    void setAttack(uint8_t newAttack) { attack = newAttack; }
    void setHealth(uint8_t newHealth) { health = (newHealth > maxHealth) ? maxHealth : newHealth; }
    void setMaxHealth(uint8_t newMaxHealth) { 
        maxHealth = newMaxHealth; 
        if (health > maxHealth) health = maxHealth;
    }
    void setSpeed(uint8_t newSpeed) { speed = newSpeed; }
    void setRange(uint8_t newRange) { range = newRange; }

    // Position methods
    std::pair<uint8_t, uint8_t> getCoordinates() const { return {x, y}; }
    bool isOnMap() const { return x != 255 && y != 255; }
    void setPosition(uint8_t newX, uint8_t newY) { x = newX; y = newY; }
    void removeFromMap() { x = 255; y = 255; }

    // Combat methods
    void takeDamage(uint8_t damage) {
        health = (damage >= health) ? 0 : health - damage;
    }

    void heal(uint8_t healAmount) {
        health = (health + healAmount > maxHealth) ? maxHealth : health + healAmount;
    }

    bool isAlive() const { return health > 0; }
    
    // Virtual method para identificar leyendas (por defecto false para Unit normal)
    virtual bool isLegend() const { return false; }

    std::string format() const override {
        return "[Unit] " + name + " (ID: " + std::to_string(id) + ")\n" +
               "Cost: " + std::to_string(cost) + " | Attack: " + std::to_string(attack) + 
               " | Health: " + std::to_string(health) + "/" + std::to_string(maxHealth) + "\n" +
               "Speed: " + std::to_string(speed) + " | Range: " + std::to_string(range) + "\n" +
               "Owner: " + std::to_string(owner) + "\n" +
               "Description: " + description;
    }
};

class Spell : public Card {
public:
    Spell(uint8_t cardId, const std::string& cardName, uint8_t cardCost, 
          const std::string& desc, PlayerId ownerId, const std::string& image = "")
        : Card(cardId, cardName, cardCost, desc, ownerId, image) {}

    std::string format() const override {
        return "[Spell] " + name + " (ID: " + std::to_string(id) + ")\n" +
               "Cost: " + std::to_string(cost) + "\n" +
               "Owner: " + std::to_string(owner) + "\n" +
               "Description: " + description;
    }
};

class Legend : public Unit {
public:
    Legend(uint8_t cardId, const std::string& cardName, uint8_t cardCost, 
           const std::string& desc, PlayerId ownerId, uint8_t atk, uint8_t hp, 
           uint8_t spd, uint8_t rng, const std::string& image = "")
        : Unit(cardId, cardName, cardCost, desc, ownerId, atk, hp, spd, rng, image) {}

    bool isLegend() const { return true; }

    std::string format() const override {
        return "[Legend] " + name + " (ID: " + std::to_string(id) + ")\n" +
               "Cost: " + std::to_string(cost) + " | Attack: " + std::to_string(attack) + 
               " | Health: " + std::to_string(health) + "/" + std::to_string(maxHealth) + "\n" +
               "Speed: " + std::to_string(speed) + " | Range: " + std::to_string(range) + "\n" +
               "Owner: " + std::to_string(owner) + "\n" +
               "Description: " + description;
    }
};
