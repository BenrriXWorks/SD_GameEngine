#include "CardLoader.hpp"
#include <fstream>
#include <print>
#include <stdexcept>
#include "../../libs/json.hpp"
#include "../effects/ComposedEffectFactory.hpp"

std::vector<CardLoader::DeckConfig> CardLoader::loadDecksFromFile(const std::string& filename) {
    std::vector<DeckConfig> decks;
    
    try {
        // Open the file
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        
        // Parse JSON
        nlohmann::json rootJson;
        file >> rootJson;
        
        // Check if the root has a "decks" property or is directly an array
        nlohmann::json decksJson;
        if (rootJson.contains("decks") && rootJson["decks"].is_array()) {
            decksJson = rootJson["decks"];
        } else if (rootJson.is_array()) {
            decksJson = rootJson;
        } else {
            throw std::runtime_error("Expected a JSON array of decks or an object with 'decks' property");
        }
        
        // Parse each deck
        for (const auto& deckJson : decksJson) {
            decks.push_back(parseDeck(deckJson));
        }
        
        std::println("Successfully loaded {} decks from {}", decks.size(), filename);
    }
    catch (const nlohmann::json::exception& e) {
        std::println("JSON parsing error: {}", e.what());
        throw;
    }
    catch (const std::exception& e) {
        std::println("Error loading decks: {}", e.what());
        throw;
    }
    
    return decks;
}

CardLoader::DeckConfig CardLoader::parseDeck(const nlohmann::json& deckJson) {
    DeckConfig deck;
    
    // Parse basic deck info
    deck.name = deckJson.at("name").get<std::string>();
    
    // Parse cards
    const auto& cardsJson = deckJson.at("cards");
    for (const auto& cardJson : cardsJson) {
        deck.cards.push_back(parseCard(cardJson));
    }
    
    return deck;
}

CardLoader::CardConfig CardLoader::parseCard(const nlohmann::json& cardJson) {
    CardConfig card;
    
    // Parse basic card info
    card.id = cardJson.at("id").get<uint32_t>();
    card.name = cardJson.at("name").get<std::string>();
    card.description = cardJson.at("description").get<std::string>();
    card.cost = cardJson.at("cost").get<uint8_t>();
    card.type = cardJson.at("type").get<std::string>();
    
    // Parse unit-specific fields if it's a unit or legend
    if (card.type == "unit" || card.type == "legend") {
        card.attack = cardJson.at("attack").get<uint8_t>();
        card.health = cardJson.at("health").get<uint8_t>();
        
        // Optional fields with defaults
        card.speed = cardJson.value("speed", 1);
        card.range = cardJson.value("range", 1);
    }
    
    // Parse effects
    if (cardJson.contains("effects") && cardJson["effects"].is_array()) {
        for (const auto& effectJson : cardJson["effects"]) {
            card.effects.push_back(parseEffect(effectJson));
        }
    }
    
    return card;
}

CardLoader::EffectConfig CardLoader::parseEffect(const nlohmann::json& effectJson) {
    EffectConfig effect;
    
    // Convertir strings a enums usando ConfigLexer
    effect.type = ConfigLexer::parseEffectType(effectJson.at("type").get<std::string>());
    effect.target_type = ConfigLexer::parseTargetType(effectJson.at("target_type").get<std::string>());
    effect.filter = ConfigLexer::parseTargetFilter(effectJson.at("filter").get<std::string>());
    effect.trigger = ConfigLexer::parseTriggerType(effectJson.at("trigger").get<std::string>());
    effect.value = effectJson.value("value", 0);
    effect.duration = effectJson.value("duration", 0);
    
    // Parse optional attribute for attribute modifiers
    if (effectJson.contains("attribute")) {
        effect.attribute = effectJson.at("attribute").get<std::string>();
    }
    
    // Parse directions for adjacency effects usando ConfigLexer
    if (effectJson.contains("directions") && effectJson["directions"].is_array()) {
        std::vector<std::string> directionStrings;
        for (const auto& dir : effectJson["directions"]) {
            directionStrings.push_back(dir.get<std::string>());
        }
        effect.directions = ConfigLexer::parseDirections(directionStrings);
    }
    
    // Parse specific position targets
    if (effectJson.contains("x")) {
        effect.x = effectJson.at("x").get<uint8_t>();
    }
    if (effectJson.contains("y")) {
        effect.y = effectJson.at("y").get<uint8_t>();
    }
    
    return effect;
}

std::vector<CardPtr> CardLoader::createCardsFromConfig(const DeckConfig& deckConfig, PlayerId owner) {
    std::vector<CardPtr> cards;
    
    for (const auto& cardConfig : deckConfig.cards) {
        CardPtr card;
        
        // Create the appropriate card type
        if (cardConfig.type == "unit") {
            auto unit = std::make_shared<Unit>(
                cardConfig.id,
                cardConfig.name,
                cardConfig.cost,
                cardConfig.description,
                owner,
                cardConfig.attack,
                cardConfig.health,
                cardConfig.speed,
                cardConfig.range
            );
            
            card = unit;
        }
        else if (cardConfig.type == "legend") {
            auto legend = std::make_shared<Legend>(
                cardConfig.id,
                cardConfig.name,
                cardConfig.cost,
                cardConfig.description,
                owner,
                cardConfig.attack,
                cardConfig.health,
                cardConfig.speed,
                cardConfig.range
            );
            
            card = legend;
        }
        else if (cardConfig.type == "spell") {
            auto spell = std::make_shared<Spell>(
                cardConfig.id,
                cardConfig.name,
                cardConfig.cost,
                cardConfig.description,
                owner
            );
            
            card = spell;
        }
        else {
            std::println("Unknown card type: {}", cardConfig.type);
            continue;
        }
        
        // Create and add effects
        for (const auto& effectConfig : cardConfig.effects) {
            auto effect = createEffectFromConfig(effectConfig, card, owner);
            if (effect) {
                card->addEffect(effect);
            }
        }
        
        cards.push_back(card);
    }
    
    return cards;
}

EffectPtr CardLoader::createEffectFromConfig(const EffectConfig& config, CardPtr source, PlayerId owner) {
    // Usar el nuevo sistema unificado de ComposedEffectFactory
    try {
        // Usar los enums directamente, con valores por defecto si es necesario
        ConfigLexer::TriggerType triggerType = config.trigger;
        ConfigLexer::TargetType targetType = config.target_type;
        ConfigLexer::TargetFilter filter = config.filter;
        
        // Crear el efecto usando la nueva factory con enums
        auto effect = ComposedEffectFactory::createFromConfig(
            config.type,      // Tipo de efecto (enum)
            triggerType,      // Tipo de trigger (enum)
            targetType,       // Tipo de objetivo (enum)
            source,           // Carta fuente
            owner,            // Propietario
            config.value,     // Valor del efecto
            filter,           // Filtro de objetivo (enum)
            config.directions,// Direcciones (vector de enums)
            config.attribute, // Atributo (string)
            config.x,         // Posición X
            config.y          // Posición Y
        );
        
        if (effect) {
            return effect;
        }
    } catch (const std::exception& e) {
        std::println("Error creating composed effect: {}", e.what());
    }
    
    // Si no se pudo crear el efecto compuesto, reportar error
    std::println("Failed to create effect");
    return nullptr;
}
