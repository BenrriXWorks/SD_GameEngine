#pragma once
#include <list>
#include <functional>
#include <optional>
#include <cstdint>
#include <string>
#include <memory>
#include "StrColors.hpp"

// Forward declaration
class Effect;

using Card = uint8_t;

class MapCell {

    public :
        enum class FloorType : uint8_t {
            NONE,
            WALKABLE
        };        
        FloorType floor;
        uint8_t x, y;
        std::list<std::shared_ptr<Effect>> passiveEffects;
        std::optional<Card> card;MapCell() = default;
        MapCell(FloorType floorType, uint8_t x, uint8_t y) : 
            floor(floorType), 
            x(x), 
            y(y), 
            passiveEffects(), // Lista vacía
            card(std::nullopt) {}

        operator std::string() const {
            using namespace StrColors;
            
            // Prioridad: carta + efecto > carta > efecto > piso
            if (card.has_value() && !passiveEffects.empty()) return MAGENTA + "⬣" + RESET; // Carta + Efecto: morado
            if (card.has_value()) return RED + "⬣" + RESET;        // Carta: rojo
            if (!passiveEffects.empty()) return BLUE + "⬣" + RESET; // Efecto: azul
            if (floor == FloorType::WALKABLE) return GREEN + "⬣" + RESET;// Caminable: verde
            
            return "⬡";     
        }

};