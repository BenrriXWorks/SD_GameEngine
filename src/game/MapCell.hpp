#pragma once
#include <functional>
#include <optional>
#include <cstdint>
#include <string>
#include <memory>
#include "../utils/StrColors.hpp"
#include "../utils/Types.hpp"
#include "../cards/Card.hpp" // Incluir definición completa de Unit

class MapCell {

    public :
        enum class FloorType : uint8_t {
            NONE,
            WALKABLE,
            SPAWN // Nuevo: posición de spawn para leyendas
        };        
        FloorType floor;
        std::optional<CardPtr> card;
        
        MapCell() = default;
        MapCell(FloorType floorType) : 
            floor(floorType), 
            card(std::nullopt) {}

        bool isSpawn() const { return floor == FloorType::SPAWN; }

        operator std::string() const {
            using namespace StrColors;
            
            if (card.has_value()) {
                auto cardPtr = card.value();
                // Verificar si es una leyenda
                if (auto unit = std::dynamic_pointer_cast<Unit>(cardPtr)) {
                    if (unit->isLegend()) {
                        return BLUE + "⬣" + RESET; // Leyenda: azul
                    }
                }
                return RED + "⬣" + RESET;        // Carta normal: rojo
            }
            if (floor == FloorType::SPAWN) return YELLOW + "⬣" + RESET; // Spawn: amarillo
            if (floor == FloorType::WALKABLE) return GREEN + "⬣" + RESET;// Caminable: verde
            
            return "⬡";     
        }

};
