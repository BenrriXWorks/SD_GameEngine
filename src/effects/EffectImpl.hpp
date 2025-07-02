#pragma once
#include "Effect.hpp"
#include "../cards/Card.hpp"

// Clase base para implementaciones concretas de efectos
class EffectImpl : public Effect {
public:
    EffectImpl(CardPtr src, PlayerId ownerId, const std::string& name)
        : Effect(src, ownerId, name) {}
        
    // Métodos específicos para implementaciones de efectos
};
