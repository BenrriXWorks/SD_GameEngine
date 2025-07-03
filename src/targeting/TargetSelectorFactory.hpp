#pragma once
#include "TargetSelectorImpl.hpp"
#include "../game/GameMap.hpp"
#include "../lex/ConfigLexer.hpp"
#include <memory>
#include <string>

class TargetSelectorFactory {
public:
    // Crear selector para la carta fuente
    static TargetSelectorPtr createSelfSelector(CardPtr source, PlayerId owner) {
        return std::make_shared<SelfTargetSelector>(source, owner);
    }
    
    // Crear selector para cartas adyacentes
    static TargetSelectorPtr createAdjacentSelector(CardPtr source, PlayerId owner, 
                                                   AdjacentTargetSelector::FilterType filter = AdjacentTargetSelector::FilterType::ALL) {
        return std::make_shared<AdjacentTargetSelector>(source, owner, filter);
    }
    
    // Crear selector para todas las cartas de un tipo
    static TargetSelectorPtr createAllCardsSelector(CardPtr source, PlayerId owner, 
                                                   AllCardsTargetSelector::FilterType filter) {
        return std::make_shared<AllCardsTargetSelector>(source, owner, filter);
    }
    
    // Crear selector para posición específica
    static TargetSelectorPtr createSpecificPositionSelector(CardPtr source, PlayerId owner, 
                                                           uint8_t x, uint8_t y) {
        return std::make_shared<SpecificPositionTargetSelector>(source, owner, x, y);
    }
    
    // Crear selector direccional
    static TargetSelectorPtr createDirectionalSelector(CardPtr source, PlayerId owner, 
                                                      GameMap::Adjacency direction,
                                                      AdjacentTargetSelector::FilterType filter = AdjacentTargetSelector::FilterType::ALL) {
        return std::make_shared<DirectionalTargetSelector>(source, owner, direction, filter);
    }
    
    // Crear selector para efectos de estado de juego
    static TargetSelectorPtr createGameStateSelector(CardPtr source, PlayerId owner) {
        return std::make_shared<GameStateTargetSelector>(source, owner);
    }
    
    // Crear selector desde configuración string
    static TargetSelectorPtr createFromConfig(const std::string& targetType, CardPtr source, PlayerId owner,
                                             const std::string& filter = "", const std::string& direction = "",
                                             uint8_t x = 0, uint8_t y = 0) {
        
        // Convertir filtro a enum
        AdjacentTargetSelector::FilterType filterType = AdjacentTargetSelector::FilterType::ALL;
        if (filter == "allies_only") {
            filterType = AdjacentTargetSelector::FilterType::ALLIES_ONLY;
        } else if (filter == "enemies_only") {
            filterType = AdjacentTargetSelector::FilterType::ENEMIES_ONLY;
        } else if (filter == "non_leaders") {
            filterType = AdjacentTargetSelector::FilterType::NON_LEADERS;
        }
        
        // Convertir dirección a enum
        GameMap::Adjacency adjDirection = GameMap::Adjacency::UP;
        if (direction == "up") {
            adjDirection = GameMap::Adjacency::UP;
        } else if (direction == "down") {
            adjDirection = GameMap::Adjacency::DOWN;
        } else if (direction == "top_left" || direction == "up_left") {
            adjDirection = GameMap::Adjacency::TOP_LEFT;
        } else if (direction == "top_right" || direction == "up_right") {
            adjDirection = GameMap::Adjacency::TOP_RIGHT;
        } else if (direction == "bottom_left" || direction == "down_left") {
            adjDirection = GameMap::Adjacency::BOTTOM_LEFT;
        } else if (direction == "bottom_right" || direction == "down_right") {
            adjDirection = GameMap::Adjacency::BOTTOM_RIGHT;
        }
        
        // Crear selector según el tipo
        if (targetType == "self") {
            return createSelfSelector(source, owner);
        } else if (targetType == "adjacent") {
            return createAdjacentSelector(source, owner, filterType);
        } else if (targetType == "all_allies") {
            return createAllCardsSelector(source, owner, AllCardsTargetSelector::FilterType::ALL_ALLIES);
        } else if (targetType == "all_enemies") {
            return createAllCardsSelector(source, owner, AllCardsTargetSelector::FilterType::ALL_ENEMIES);
        } else if (targetType == "all_units") {
            return createAllCardsSelector(source, owner, AllCardsTargetSelector::FilterType::ALL_UNITS);
        } else if (targetType == "all_cards") {
            return createAllCardsSelector(source, owner, AllCardsTargetSelector::FilterType::ALL_CARDS);
        } else if (targetType == "specific_position") {
            return createSpecificPositionSelector(source, owner, x, y);
        } else if (targetType == "directional") {
            return createDirectionalSelector(source, owner, adjDirection, filterType);
        } else if (targetType == "game_state") {
            return createGameStateSelector(source, owner);
        }
        
        // Valor por defecto
        return createSelfSelector(source, owner);
    }
    
    // Método para crear selector usando enums (nueva versión)
    static TargetSelectorPtr createFromConfig(ConfigLexer::TargetType targetType, CardPtr source, PlayerId owner,
                                             ConfigLexer::TargetFilter filter = ConfigLexer::TargetFilter::ANY,
                                             const std::vector<GameMap::Adjacency>& directions = {},
                                             uint8_t x = 0, uint8_t y = 0) {
        (void)directions; // Suppress unused parameter warning
        
        // Convertir filtro de ConfigLexer a AdjacentTargetSelector::FilterType
        AdjacentTargetSelector::FilterType filterType = AdjacentTargetSelector::FilterType::ALL;
        switch (filter) {
            case ConfigLexer::TargetFilter::ALLIES_ONLY:
                filterType = AdjacentTargetSelector::FilterType::ALLIES_ONLY;
                break;
            case ConfigLexer::TargetFilter::ENEMIES_ONLY:
                filterType = AdjacentTargetSelector::FilterType::ENEMIES_ONLY;
                break;
            case ConfigLexer::TargetFilter::ANY:
                filterType = AdjacentTargetSelector::FilterType::ALL;
                break;
        }
        
        // Crear selector según el tipo
        switch (targetType) {
            case ConfigLexer::TargetType::SELF:
                return createSelfSelector(source, owner);
            case ConfigLexer::TargetType::ADJACENT:
                return createAdjacentSelector(source, owner, filterType);
            case ConfigLexer::TargetType::ALL_ALLIES:
                return createAllCardsSelector(source, owner, AllCardsTargetSelector::FilterType::ALL_ALLIES);
            case ConfigLexer::TargetType::ALL_ENEMIES:
                return createAllCardsSelector(source, owner, AllCardsTargetSelector::FilterType::ALL_ENEMIES);
            case ConfigLexer::TargetType::ATTACK_TARGET:
                return createSelfSelector(source, owner); // Simplificado por ahora
            case ConfigLexer::TargetType::ATTACKER:
                return createSelfSelector(source, owner); // Simplificado por ahora
            case ConfigLexer::TargetType::SPECIFIC_POSITION:
                return createSpecificPositionSelector(source, owner, x, y);
            case ConfigLexer::TargetType::GAME_STATE:
                return createGameStateSelector(source, owner);
        }
        
        // Valor por defecto
        return createSelfSelector(source, owner);
    }
};
