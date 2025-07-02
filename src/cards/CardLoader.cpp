#include "CardLoader.hpp"
#include <fstream>
#include <print>
#include <stdexcept>
#include "../../libs/json.hpp"
#include "../effects/EffectDispatch.hpp"

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
    
    effect.type = effectJson.at("type").get<std::string>();
    effect.target = effectJson.at("target").get<std::string>();
    
    // Optional fields
    if (effectJson.contains("value")) {
        effect.value = effectJson["value"].get<int>();
    }
    
    if (effectJson.contains("trigger")) {
        effect.trigger = effectJson["trigger"].get<std::string>();
    }
    
    if (effectJson.contains("direction")) {
        effect.direction = effectJson["direction"].get<std::string>();
    }
    
    if (effectJson.contains("position")) {
        auto position = effectJson["position"];
        effect.x = position["x"].get<uint8_t>();
        effect.y = position["y"].get<uint8_t>();
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
    // Use the optimized trie-based dispatch system
    auto creator = EffectDispatch::lookup(config.type);
    
    if (creator) {
        return creator(config, source, owner);
    } else {
        std::println("Unknown effect type: {}", config.type);
        return nullptr;
    }
}
