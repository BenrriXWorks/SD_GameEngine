#include <print>
#include <array>
#include <functional>
#include <utility>
#include <string>
#include <vector>
#include "src/game/GameState.hpp"
#include "src/effects/Effect.hpp"
#include "src/game/MapCell.hpp"
#include "src/game/GameMap.hpp"
#include "src/effects/EffectStack.hpp"
#include "src/effects/impl/PrintEffect.hpp"
#include "src/cards/CardLoader.hpp"

using namespace std;

void printPlayerInfo(const Player& player, const string& playerName) {
    println("=== {} ===", playerName);
    println("ID: {}, Team: {}, Actions Left: {}/{}", 
        player.id, 
        static_cast<uint8_t>(player.team), 
        player.actionsLeft, 
        player.maxActions);
    
    print("Hand ({}): [", player.hand.size());
    for (size_t i = 0; i < player.hand.size(); ++i) {
        if (player.hand[i])
            print("{}{}", player.hand[i]->getName(), (i < player.hand.size() - 1) ? ", " : "");
        else
            print("(null){}", (i < player.hand.size() - 1) ? ", " : "");
    }
    println("]");
    
    println("Deck Cards Remaining: {}", player.deck.size());
    println("Leader Card: {}", player.leader ? player.leader->getName() : "(none)");
    println();
}

void printGameState(const GameState& game) {
    println("=== GAME STATE ===");
    println("Turn: {}, Current Team: {}, Phase: {}", 
        game.turn, 
        static_cast<uint8_t>(game.currentTeamInTurn),
        static_cast<uint8_t>(game.currentPhase));
    println();
    
    printPlayerInfo(game.players[0], "Player 0 (Team A)");
    printPlayerInfo(game.players[1], "Player 1 (Team B)");
    
    println("Game Map:");
    println("{}", string(game.gameMap));
    println();
}

int main() {
    println("=== PRUEBA DEL SISTEMA DE JUEGO DE CARTAS ===\n");

    // Paso 1: Cargar barajas desde el JSON
    println("Cargando barajas desde decks.json...");
    std::vector<CardLoader::DeckConfig> deckConfigs = CardLoader::loadDecksFromFile("decks.json");
    if (deckConfigs.size() < 2) {
        println("Error: Se requieren al menos dos barajas en el archivo JSON.");
        return 1;
    }
    // Mostrar nombres y cantidad de cartas
    for (size_t i = 0; i < deckConfigs.size(); ++i) {
        println("Deck {}: '{}' ({} cartas)", i, deckConfigs[i].name, deckConfigs[i].cards.size());
    }

    // Paso 2: Crear cartas para cada jugador
    std::vector<CardPtr> deck1 = CardLoader::createCardsFromConfig(deckConfigs[0], 0);
    std::vector<CardPtr> deck2 = CardLoader::createCardsFromConfig(deckConfigs[1], 1);

    // Inicializar GameState con las barajas cargadas
    println("Inicializando GameState con las barajas cargadas...");
    GameState game(deck1, deck2);
    printGameState(game);
    
    // Test 3: Robar cartas
    println("Test 3: Probando robo de cartas...");
    auto currentPlayer = game.getCurrentPlayer();
    if (currentPlayer) {
        println("Jugador actual (ID: {}) robando carta...", currentPlayer->id);
        size_t handSizeBefore = currentPlayer->hand.size();
        size_t deckSizeBefore = currentPlayer->deck.size();
        
        bool drawSuccess = currentPlayer->drawCard();
        println("Robo exitoso: {}", drawSuccess ? "SI" : "NO");
        println("Mano antes: {}, después: {}", handSizeBefore, currentPlayer->hand.size());
        println("Baraja antes: {}, después: {}", deckSizeBefore, currentPlayer->deck.size());
        println();
    }
    
    // Test 4: Jugar carta desde la mano
    println("Test 4: Jugando carta desde la mano...");
    currentPlayer = game.getCurrentPlayer();
    if (currentPlayer && !currentPlayer->hand.empty()) {
        CardPtr cardToPlay = currentPlayer->hand[0];
        println("Jugando carta {} en posición (1, 3)...", cardToPlay ? cardToPlay->getName() : "(null)");
        
        bool playSuccess = game.playCardFromHand(currentPlayer->id, cardToPlay, 1, 3);
        println("Juego exitoso: {}", playSuccess ? "SI" : "NO");
        
        if (playSuccess) {
            println("Carta colocada en el mapa:");
            auto cell = game.gameMap.at(1, 3);
            if (cell && cell->card.has_value() && cell->card.value()) {
                println("Celda (1,3) contiene carta: {}", cell->card.value()->getName());
            }
        }
        
        printPlayerInfo(*currentPlayer, "Jugador después de jugar carta");
    }
    
    // Test 5: Probar efectos
    println("Test 5: Probando sistema de efectos...");
    auto& effectStack = game.effectStack;
    
    // Crear cartas de prueba para los efectos (usar cartas reales del deck)
    CardPtr testCard1 = !deck1.empty() ? deck1[0] : nullptr;
    CardPtr testCard2 = !deck2.empty() ? deck2[0] : nullptr;
    
    // Agregar efectos de prueba
    if (testCard1)
        effectStack.addEffect(TriggerAction::ON_PLAY, 1, 
            make_shared<PrintEffect>(testCard1, 0, "Efecto de Invocación"));
    if (testCard2)
        effectStack.addEffect(TriggerAction::ON_ATTACK, 2, 
            make_shared<PrintEffect>(testCard2, 1, "Efecto de Ataque"));
    
    println("Stack de efectos:");
    println("{}", string(effectStack));
    
    // Disparar efectos ON_PLAY
    println("Disparando efectos ON_PLAY...");
    auto sourceCell = game.gameMap.at(2, 3);
    list<MapCell*> targets = {sourceCell};
    effectStack.triggerEffects(TriggerAction::ON_PLAY, game.gameMap, sourceCell, targets);
    
    println("Stack después de disparar ON_PLAY:");
    println("{}", string(effectStack));
    
    // Test 6: Cambio de turno
    println("Test 6: Probando cambio de turno...");
    Team teamBefore = game.currentTeamInTurn;
    uint32_t turnBefore = game.turn;
    
    game.endTurn();
    
    println("Equipo antes: {}, después: {}", 
        static_cast<uint8_t>(teamBefore), 
        static_cast<uint8_t>(game.currentTeamInTurn));
    println("Turno antes: {}, después: {}", turnBefore, game.turn);
    
    printGameState(game);
    
    // Test 7: Múltiples robos de cartas hasta llenar la mano
    println("Test 7: Llenando mano hasta el máximo...");
    currentPlayer = game.getCurrentPlayer();
    if (currentPlayer) {
        while (currentPlayer->hand.size() < currentPlayer->maxHandSize && !currentPlayer->deck.empty()) {
            currentPlayer->drawCard();
        }
        printPlayerInfo(*currentPlayer, "Jugador con mano llena");
        
        // Intentar robar una carta más (debería fallar)
        bool extraDraw = currentPlayer->drawCard();
        println("Intento de robar carta extra (mano llena): {}", extraDraw ? "EXITOSO" : "FALLÓ (correcto)");
    }
    
    println("\n=== PRUEBAS COMPLETADAS EXITOSAMENTE ===");
    return 0;
}