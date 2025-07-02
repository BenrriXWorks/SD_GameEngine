#pragma once
#include <cstdint>
#include <memory>

using PlayerId = uint32_t;

// Forward declarations
class Card;
class Unit;
class Spell;
class Effect;
class GameMap;
class MapCell;

// Define shared pointers
using CardPtr = std::shared_ptr<Card>;
using EffectPtr = std::shared_ptr<Effect>;
