#include "src/cards/CardLoader.hpp"
#include "src/game/GameState.hpp"
#include "src/utils/StrColors.hpp"
#include <print>
#include <iomanip>
#include <list>
#include <functional>

// Define tipos de acciones para tests
enum class TestActionType {
    PLAY_CARD,
    MOVE_CARD,
    ATTACK,
    END_TURN,
    CHECK_CONDITION
};

// Estructura para acciones de prueba
struct TestAction {
    TestActionType type;
    PlayerId playerId;
    size_t cardIndex = 0;           // Índice de la carta a jugar (para PLAY_CARD)
    uint8_t x = 0, y = 0;        // Posición para colocar/mover
    uint8_t targetX = 0, targetY = 0; // Posición objetivo para ataque
    std::string description;     // Descripción de la acción
    std::function<bool(GameState&)> condition; // Condición a verificar (para CHECK_CONDITION)
    
    // Constructor para PLAY_CARD
    static TestAction PlayCard(PlayerId id, size_t index, uint8_t posX, uint8_t posY, const std::string& desc) {
        TestAction action;
        action.type = TestActionType::PLAY_CARD;
        action.playerId = id;
        action.cardIndex = index;
        action.x = posX;
        action.y = posY;
        action.description = desc;
        return action;
    }
    
    // Constructor para MOVE_CARD
    static TestAction MoveCard(PlayerId id, uint8_t fromX, uint8_t fromY, uint8_t toX, uint8_t toY, const std::string& desc) {
        TestAction action;
        action.type = TestActionType::MOVE_CARD;
        action.playerId = id;
        action.x = fromX;
        action.y = fromY;
        action.targetX = toX;
        action.targetY = toY;
        action.description = desc;
        return action;
    }
    
    // Constructor para ATTACK
    static TestAction Attack(PlayerId id, uint8_t fromX, uint8_t fromY, uint8_t targetX, uint8_t targetY, const std::string& desc) {
        TestAction action;
        action.type = TestActionType::ATTACK;
        action.playerId = id;
        action.x = fromX;
        action.y = fromY;
        action.targetX = targetX;
        action.targetY = targetY;
        action.description = desc;
        return action;
    }
    
    // Constructor para END_TURN
    static TestAction EndTurn(PlayerId id, const std::string& desc) {
        TestAction action;
        action.type = TestActionType::END_TURN;
        action.playerId = id;
        action.description = desc;
        return action;
    }
    
    // Constructor para CHECK_CONDITION
    static TestAction CheckCondition(const std::string& desc, std::function<bool(GameState&)> checkFunc) {
        TestAction action;
        action.type = TestActionType::CHECK_CONDITION;
        action.description = desc;
        action.condition = checkFunc;
        return action;
    }
};

void printCard(const CardPtr& card) {
    std::println("{0}Card ID: {1}{2}", StrColors::YELLOW, card->getId(), StrColors::RESET);
    std::println("Name: {0}{1}{2}", StrColors::GREEN, card->getName(), StrColors::RESET);
    std::println("Cost: {0}{1}{2}", StrColors::CYAN, (int)card->getCost(), StrColors::RESET);
    std::println("Description: {}", card->getDescription());
    
    if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
        std::println("Type: {0}Unit{1}", StrColors::BLUE, StrColors::RESET);
        std::println("Attack: {0}{1}{2} | Health: {3}{4}{5} | Speed: {6}{7}{8} | Range: {9}{10}{11}", 
                  StrColors::RED, (int)unit->getAttack(), StrColors::RESET,
                  StrColors::GREEN, (int)unit->getHealth(), StrColors::RESET,
                  StrColors::YELLOW, (int)unit->getSpeed(), StrColors::RESET,
                  StrColors::CYAN, (int)unit->getRange(), StrColors::RESET);
    } else if (auto spell = std::dynamic_pointer_cast<Spell>(card)) {
        std::println("Type: {0}Spell{1}", StrColors::MAGENTA, StrColors::RESET);
    }
    
    std::println("Effects: {}", (int)card->getEffects().size());
    std::println("---");
}

void printDeck(const std::vector<CardPtr>& deck, const std::string& deckName) {
    std::println("");
    std::println("========================================");
    std::println("{0}DECK: {1}{2}", StrColors::BOLD, deckName, StrColors::RESET);
    std::println("Total cards: {}", deck.size());
    std::println("========================================");
    
    int unitCount = 0;
    int spellCount = 0;
    int totalCost = 0;
    
    for (const auto& card : deck) {
        printCard(card);
        
        if (std::dynamic_pointer_cast<Unit>(card)) {
            unitCount++;
        } else if (std::dynamic_pointer_cast<Spell>(card)) {
            spellCount++;
        }
        
        totalCost += card->getCost();
    }
    
    std::println("");
    std::println("SUMMARY:");
    std::println("Units: {0}{1}{2}", StrColors::GREEN, unitCount, StrColors::RESET);
    std::println("Spells: {0}{1}{2}", StrColors::MAGENTA, spellCount, StrColors::RESET);
    std::println("Total Cost: {0}{1}{2}", StrColors::YELLOW, totalCost, StrColors::RESET);
    std::println("Average Cost: {0}{1:.1f}{2}", StrColors::CYAN, 
              (deck.empty() ? 0.0 : static_cast<double>(totalCost) / deck.size()), StrColors::RESET);
}

void testGameWithLoadedDecks() {
    std::println("{0}=== TESTING GAME WITH LOADED DECKS ==={1}", StrColors::BOLD, StrColors::RESET);
    
    try {
        // Load all decks
        std::vector<CardLoader::DeckConfig> deckConfigs = CardLoader::loadDecksFromFile("decks.json");
        
        if (deckConfigs.size() < 2) {
            std::println("{0}Error: Need at least 2 decks to test a game!{1}", StrColors::RED, StrColors::RESET);
            return;
        }
        
        std::println("Successfully loaded {} decks from decks.json", deckConfigs.size());
        
        // Convert to CardPtr vectors
        std::vector<std::vector<CardPtr>> gameDecks;
        for (const auto& deckConfig : deckConfigs) {
            std::vector<CardPtr> deck = CardLoader::createCardsFromConfig(deckConfig, 0);
            gameDecks.push_back(deck);
        }
        
        // Create a simple game state
        GameState gameState;
        
        // Add two players to the game
        gameState.addPlayer(0, Team::TEAM_A, "Player 0");
        gameState.addPlayer(1, Team::TEAM_B, "Player 1");
        
        // Set player decks
        gameState.setPlayerDeck(0, gameDecks[0]);
        gameState.setPlayerDeck(1, gameDecks[1]);
        
        // Start the game - this will shuffle decks and draw initial hands
        gameState.startGame();
        
        // Simulate a simple turn
        std::println("Turn 1: Player 0 plays a card...");
        
        // Get player's hand and play first card if it's a unit
        const auto& player = gameState.getPlayer(0);
        if (!player.hand.empty()) {
            auto cardToPlay = player.hand[0]; // Take the first card from hand
            std::println("Attempting to play: {0}{1}{2}", StrColors::GREEN, cardToPlay->getName(), StrColors::RESET);
            
            // Try to place card on the map (if it's a Unit)
            if (auto unit = std::dynamic_pointer_cast<Unit>(cardToPlay)) {
                // Try to place the unit at position (2, 3) for player 0
                MapCell* cell = gameState.getMap().at(2, 3);
                if (cell && !cell->card.has_value()) {
                    gameState.playCard(0, cardToPlay, 2, 3);
                    std::println("✓ Successfully placed unit at (2, 3)");
                    
                    // Apply any effects the card might have
                    std::println("Effects applied: {}", cardToPlay->getEffects().size());
                    
                    // Show the map state
                    std::println("Map state after placement:\n{}", (std::string) gameState.getMap());
                } else {
                    std::println("{0}✗ Position (2, 3) is not empty!{1}", StrColors::RED, StrColors::RESET);
                }
            } else {
                std::println("{0}✗ Card is not a unit, cannot place on map!{1}", StrColors::RED, StrColors::RESET);
            }
        } else {
            std::println("{0}✗ Player's hand is empty!{1}", StrColors::RED, StrColors::RESET);
        }
        
        std::println("Game test completed!");
        
    } catch (const std::exception& e) {
        std::println("{0}Error during test: {1}{2}", StrColors::RED, e.what(), StrColors::RESET);
    }
}

void runTestActions(std::list<TestAction> actions, GameState& gameState) {        
    std::println("{}=== EJECUTANDO SECUENCIA DE ACCIONES DE TEST ==={}", StrColors::BOLD, StrColors::RESET);
    
    try {
        std::println("Estado inicial del juego:");
        std::println("Turno: {}, Jugador actual: {}", gameState.getTurnNumber(), gameState.getCurrentPlayer());
        std::println("Acciones restantes: {}", gameState.getActionsRemaining(gameState.getCurrentPlayer()));
        std::println("Mapa inicial:\n{}", (std::string) gameState.getMap());
        
        // Ejecutar todas las acciones en secuencia
        int actionNumber = 1;
        for (const auto& action : actions) {
            std::println("\n{}Acción #{}: {}{}", StrColors::CYAN, actionNumber++, action.description, StrColors::RESET);
            
            bool success = false;
            
            switch (action.type) {
                case TestActionType::PLAY_CARD: {
                    const auto& player = gameState.getPlayer(action.playerId);
                    if (player.hand.size() <= action.cardIndex) {
                        std::println("{}✗ El jugador {} no tiene suficientes cartas en mano!{}", 
                                   StrColors::RED, action.playerId, StrColors::RESET);
                        continue;
                    }
                    
                    auto cardToPlay = player.hand[action.cardIndex];
                    std::println("Intentando jugar: {}{}{}", StrColors::GREEN, cardToPlay->getName(), StrColors::RESET);
                    
                    if (auto unit = std::dynamic_pointer_cast<Unit>(cardToPlay)) {
                        MapCell* cell = gameState.getMap().at(action.x, action.y);
                        if (cell && !cell->card.has_value()) {
                            gameState.playCard(action.playerId, cardToPlay, action.x, action.y);
                            std::println("✓ Carta colocada en posición ({}, {})", action.x, action.y);
                            std::println("Efectos aplicados: {}", cardToPlay->getEffects().size());
                            success = true;
                        } else {
                            std::println("{}✗ La posición ({}, {}) no está disponible!{}", 
                                       StrColors::RED, action.x, action.y, StrColors::RESET);
                        }
                    } else {
                        std::println("{}✗ La carta no es una unidad!{}", StrColors::RED, StrColors::RESET);
                    }
                    break;
                }
                
                case TestActionType::MOVE_CARD: {
                    // Encontrar la carta en la posición inicial
                    MapCell* cell = gameState.getMap().at(action.x, action.y);
                    if (!cell || !cell->card.has_value()) {
                        std::println("{}✗ No hay carta en posición ({}, {})!{}", 
                                   StrColors::RED, action.x, action.y, StrColors::RESET);
                        continue;
                    }
                    
                    CardPtr cardToMove = cell->card.value();
                    std::println("Intentando mover {}{}{} desde ({}, {}) a ({}, {})", 
                               StrColors::GREEN, cardToMove->getName(), StrColors::RESET,
                               action.x, action.y, action.targetX, action.targetY);
                    
                    success = gameState.moveCard(action.playerId, cardToMove, action.targetX, action.targetY);
                    if (success) {
                        std::println("✓ Carta movida correctamente");
                    } else {
                        std::println("{}✗ No se pudo mover la carta!{}", StrColors::RED, StrColors::RESET);
                    }
                    break;
                }
                
                case TestActionType::ATTACK: {
                    // Encontrar la carta atacante
                    MapCell* fromCell = gameState.getMap().at(action.x, action.y);
                    if (!fromCell || !fromCell->card.has_value()) {
                        std::println("{}✗ No hay carta atacante en ({}, {})!{}", 
                                   StrColors::RED, action.x, action.y, StrColors::RESET);
                        continue;
                    }
                    
                    CardPtr attacker = fromCell->card.value();
                    std::println("Atacando con {}{}{} desde ({}, {}) a ({}, {})", 
                               StrColors::GREEN, attacker->getName(), StrColors::RESET,
                               action.x, action.y, action.targetX, action.targetY);
                    
                    success = gameState.attackWithCard(action.playerId, attacker, action.targetX, action.targetY);
                    if (success) {
                        std::println("✓ Ataque realizado correctamente");
                    } else {
                        std::println("{}✗ No se pudo realizar el ataque!{}", StrColors::RED, StrColors::RESET);
                    }
                    break;
                }
                
                case TestActionType::END_TURN: {
                    std::println("Jugador {} finaliza su turno", action.playerId);
                    gameState.endTurn(action.playerId);
                    success = true;
                    break;
                }
                
                case TestActionType::CHECK_CONDITION: {
                    if (action.condition) {
                        success = action.condition(gameState);
                        if (success) {
                            std::println("✓ Condición verificada correctamente");
                        } else {
                            std::println("{}✗ La condición no se cumple!{}", StrColors::RED, StrColors::RESET);
                        }
                    }
                    break;
                }
            }
            
            // Mostrar estado actualizado después de cada acción
            std::println("\nEstado actual:");
            std::println("Turno: {}, Jugador actual: {}", gameState.getTurnNumber(), gameState.getCurrentPlayer());
            std::println("Acciones restantes: {}", gameState.getActionsRemaining(gameState.getCurrentPlayer()));
            std::println("Mapa actual:\n{}", (std::string) gameState.getMap());
        }
        
        std::println("\n{}Secuencia de test completada!{}", StrColors::GREEN, StrColors::RESET);
        
    } catch (const std::exception& e) {
        std::println("{}Error durante el test: {}{}", StrColors::RED, e.what(), StrColors::RESET);
    }
}

void testAdvancedGameSequence() {
    std::println("{}=== TEST AVANZADO DE SECUENCIA DE JUEGO ==={}", StrColors::BOLD, StrColors::RESET);
    
    try {
        // Cargar mazos
        std::vector<CardLoader::DeckConfig> deckConfigs = CardLoader::loadDecksFromFile("decks.json");
        
        if (deckConfigs.size() < 2) {
            std::println("{}Error: Se necesitan al menos 2 mazos para el test!{}", StrColors::RED, StrColors::RESET);
            return;
        }
        
        // Crear estado de juego
        GameState gameState;
        
        // Agregar jugadores
        gameState.addPlayer(0, Team::TEAM_A, "Jugador 0");
        gameState.addPlayer(1, Team::TEAM_B, "Jugador 1");
        
        // Convertir y asignar mazos
        std::vector<std::vector<CardPtr>> gameDecks;
        for (const auto& deckConfig : deckConfigs) {
            std::vector<CardPtr> deck = CardLoader::createCardsFromConfig(deckConfig, 0);
            gameDecks.push_back(deck);
        }
        
        // Asegurar que el primer jugador tenga unidades en su mano
        std::vector<CardPtr> deckWithUnits;
        for (const auto& card : gameDecks[0]) {
            if (std::dynamic_pointer_cast<Unit>(card)) {
                deckWithUnits.push_back(card);
            }
        }
        // Añadir algunas unidades al principio del mazo para garantizar que se robe una
        for (int i = 0; i < 5 && i < static_cast<int>(deckWithUnits.size()); ++i) {
            gameDecks[0].insert(gameDecks[0].begin(), deckWithUnits[i]);
        }
        
        // Asegurar que el segundo jugador tenga unidades en su mano
        deckWithUnits.clear();
        for (const auto& card : gameDecks[1]) {
            if (std::dynamic_pointer_cast<Unit>(card)) {
                deckWithUnits.push_back(card);
            }
        }
        // Añadir algunas unidades al principio del mazo para garantizar que se robe una
        for (int i = 0; i < 5 && i < static_cast<int>(deckWithUnits.size()); ++i) {
            gameDecks[1].insert(gameDecks[1].begin(), deckWithUnits[i]);
        }
        
        // Establecer mazos
        gameState.setPlayerDeck(0, gameDecks[0]);
        gameState.setPlayerDeck(1, gameDecks[1]);
        
        // Iniciar juego
        gameState.startGame();
        
        // Crear lista de acciones para el test
        std::list<TestAction> actions;
        
        // Encontrar una unidad en la mano del jugador 0
        size_t unitIndexPlayer0 = SIZE_MAX;
        const auto& player0 = gameState.getPlayer(0);
        for (size_t i = 0; i < player0.hand.size(); ++i) {
            if (std::dynamic_pointer_cast<Unit>(player0.hand[i])) {
                unitIndexPlayer0 = i;
                std::println("Jugador 0 tiene una unidad en la posición {} de su mano: {}", 
                           i, player0.hand[i]->getName());
                break;
            }
        }
        
        // Encontrar una unidad en la mano del jugador 1
        size_t unitIndexPlayer1 = SIZE_MAX;
        const auto& player1 = gameState.getPlayer(1);
        for (size_t i = 0; i < player1.hand.size(); ++i) {
            if (std::dynamic_pointer_cast<Unit>(player1.hand[i])) {
                unitIndexPlayer1 = i;
                std::println("Jugador 1 tiene una unidad en la posición {} de su mano: {}", 
                           i, player1.hand[i]->getName());
                break;
            }
        }
        
        if (unitIndexPlayer0 == SIZE_MAX || unitIndexPlayer1 == SIZE_MAX) {
            std::println("{}Error: Ambos jugadores deben tener al menos una unidad en su mano para el test!{}", 
                       StrColors::RED, StrColors::RESET);
            return;
        }
        
        // Secuencia de acciones para el test
        actions.push_back(TestAction::PlayCard(0, unitIndexPlayer0, 2, 3, "Jugador 0 coloca una unidad en (2,3)"));
        
        // Verificar que se consumió una acción
        actions.push_back(TestAction::CheckCondition("Verificar acciones restantes después de jugar", 
            [](GameState& state) -> bool {
                return state.getActionsRemaining(0) == 2; // Debería tener 2 acciones restantes de 3
            }
        ));
        
        // Buscar otra unidad para el jugador 0 (si hay)
        size_t secondUnitIndexPlayer0 = SIZE_MAX;
        for (size_t i = 0; i < player0.hand.size(); ++i) {
            if (i != unitIndexPlayer0 && std::dynamic_pointer_cast<Unit>(player0.hand[i])) {
                secondUnitIndexPlayer0 = i;
                std::println("Jugador 0 tiene otra unidad en la posición {} de su mano: {}", 
                           i, player0.hand[i]->getName());
                break;
            }
        }
        
        // Si hay otra unidad, colocarla
        if (secondUnitIndexPlayer0 != SIZE_MAX) {
            // Compensar por la carta que se acaba de jugar
            if (secondUnitIndexPlayer0 > unitIndexPlayer0) {
                secondUnitIndexPlayer0 -= 1;
            }
            actions.push_back(TestAction::PlayCard(0, secondUnitIndexPlayer0, 3, 3, 
                                               "Jugador 0 coloca una segunda unidad en (3,3)"));
            
            // Verificar que se consumió otra acción
            actions.push_back(TestAction::CheckCondition("Verificar acciones restantes después de jugar segunda carta", 
                [](GameState& state) -> bool {
                    return state.getActionsRemaining(0) == 1; // Debería tener 1 acción restante
                }
            ));
            
            // Mover la primera carta
            actions.push_back(TestAction::MoveCard(0, 2, 3, 2, 4, "Jugador 0 mueve la unidad de (2,3) a (2,4)"));
            
            // Verificar que se consumió la última acción
            actions.push_back(TestAction::CheckCondition("Verificar que no quedan acciones", 
                [](GameState& state) -> bool {
                    return state.getActionsRemaining(0) == 0; // No debería tener más acciones
                }
            ));
        } else {
            std::println("Jugador 0 no tiene una segunda unidad en mano, algunas pruebas se omitirán");
        }
        
        // Terminar turno del jugador 0
        actions.push_back(TestAction::EndTurn(0, "Jugador 0 termina su turno"));
        
        // Verificar que ahora es el turno del jugador 1
        actions.push_back(TestAction::CheckCondition("Verificar que es turno del jugador 1", 
            [](GameState& state) -> bool {
                return state.getCurrentPlayer() == 1;
            }
        ));
        
        // Jugador 1 coloca una carta
        actions.push_back(TestAction::PlayCard(1, unitIndexPlayer1, 3, 2, "Jugador 1 coloca una unidad en (3,2)"));
        
        // Verificar que el jugador 1 tiene 2 acciones restantes
        actions.push_back(TestAction::CheckCondition("Verificar acciones de jugador 1", 
            [](GameState& state) -> bool {
                return state.getActionsRemaining(1) == 2;
            }
        ));
        
        // Terminar turno del jugador 1
        actions.push_back(TestAction::EndTurn(1, "Jugador 1 termina su turno"));
        
        // Verificar que vuelve a ser turno del jugador 0
        actions.push_back(TestAction::CheckCondition("Verificar que vuelve a ser turno del jugador 0", 
            [](GameState& state) -> bool {
                return state.getCurrentPlayer() == 0;
            }
        ));
        
        // Verificar que el jugador 0 tiene todas sus acciones de nuevo
        actions.push_back(TestAction::CheckCondition("Verificar que jugador 0 tiene todas sus acciones de nuevo", 
            [](GameState& state) -> bool {
                return state.getActionsRemaining(0) == 3; // Debería tener 3 acciones de nuevo
            }
        ));
        
        // Ahora ejecutar la secuencia de acciones
        runTestActions(actions, gameState);
        
    } catch (const std::exception& e) {
        std::println("{}Error durante el test: {}{}", StrColors::RED, e.what(), StrColors::RESET);
    }
}

void testLegendSystemAndSpawns() {
    std::println("{}=== TEST DEL SISTEMA DE LEYENDAS Y SPAWNS ==={}", StrColors::BOLD, StrColors::RESET);
    
    try {
        // Cargar mazos
        std::vector<CardLoader::DeckConfig> deckConfigs = CardLoader::loadDecksFromFile("decks.json");
        
        if (deckConfigs.size() < 2) {
            std::println("{}Error: Se necesitan al menos 2 mazos para el test!{}", StrColors::RED, StrColors::RESET);
            return;
        }
        
        // Crear estado de juego
        GameState gameState;
        
        // Agregar jugadores
        gameState.addPlayer(0, Team::TEAM_A, "Jugador 0");
        gameState.addPlayer(1, Team::TEAM_B, "Jugador 1");
        
        // Convertir y asignar mazos
        std::vector<std::vector<CardPtr>> gameDecks;
        for (const auto& deckConfig : deckConfigs) {
            std::vector<CardPtr> deck = CardLoader::createCardsFromConfig(deckConfig, 0);
            gameDecks.push_back(deck);
        }
        
        // Asignar mazos a jugadores
        gameState.setPlayerDeck(0, gameDecks[0]);
        gameState.setPlayerDeck(1, gameDecks[1]);
        
        // Mostrar mapa inicial con spawn points
        std::println("\n{}Mapa inicial con spawn points:{}", StrColors::CYAN, StrColors::RESET);
        std::println("{}", (std::string) gameState.getMap());
        
        // Iniciar juego (esto debería colocar automáticamente las leyendas)
        std::println("\n{}Iniciando juego - las leyendas deberían aparecer automáticamente en spawn points...{}", 
                   StrColors::YELLOW, StrColors::RESET);
        gameState.startGame();
        
        // Verificar que las leyendas se colocaron correctamente
        std::println("\n{}Mapa después de iniciar el juego:{}", StrColors::CYAN, StrColors::RESET);
        std::println("{}", (std::string) gameState.getMap());
        
        // Verificar spawn positions específicamente
        auto spawnPos0 = gameState.getMap().getSpawnPosition(0);
        auto spawnPos1 = gameState.getMap().getSpawnPosition(1);
        
        std::println("\n{}Verificando posiciones de spawn:{}", StrColors::YELLOW, StrColors::RESET);
        std::println("Spawn Jugador 0: ({}, {})", spawnPos0.first, spawnPos0.second);
        std::println("Spawn Jugador 1: ({}, {})", spawnPos1.first, spawnPos1.second);
        
        // Verificar que hay cartas en las posiciones de spawn
        MapCell* spawn0Cell = gameState.getMap().at(spawnPos0.first, spawnPos0.second);
        MapCell* spawn1Cell = gameState.getMap().at(spawnPos1.first, spawnPos1.second);
        
        if (spawn0Cell && spawn0Cell->card.has_value()) {
            auto card0 = spawn0Cell->card.value();
            auto legend0 = std::dynamic_pointer_cast<Legend>(card0);
            if (legend0) {
                std::println("✓ Leyenda del Jugador 0: {}{}{} en spawn position", 
                           StrColors::GREEN, legend0->getName(), StrColors::RESET);
                std::println("  Stats: ATK={}, HP={}, SPD={}, RNG={}", 
                           legend0->getAttack(), legend0->getHealth(), 
                           legend0->getSpeed(), legend0->getRange());
            } else {
                std::println("{}✗ Carta en spawn 0 no es una leyenda!{}", StrColors::RED, StrColors::RESET);
            }
        } else {
            std::println("{}✗ No hay carta en spawn position del Jugador 0!{}", StrColors::RED, StrColors::RESET);
        }
        
        if (spawn1Cell && spawn1Cell->card.has_value()) {
            auto card1 = spawn1Cell->card.value();
            auto legend1 = std::dynamic_pointer_cast<Legend>(card1);
            if (legend1) {
                std::println("✓ Leyenda del Jugador 1: {}{}{} en spawn position", 
                           StrColors::GREEN, legend1->getName(), StrColors::RESET);
                std::println("  Stats: ATK={}, HP={}, SPD={}, RNG={}", 
                           legend1->getAttack(), legend1->getHealth(), 
                           legend1->getSpeed(), legend1->getRange());
            } else {
                std::println("{}✗ Carta en spawn 1 no es una leyenda!{}", StrColors::RED, StrColors::RESET);
            }
        } else {
            std::println("{}✗ No hay carta en spawn position del Jugador 1!{}", StrColors::RED, StrColors::RESET);
        }
        
        // Verificar estado de jugadores
        std::println("\n{}Estado de los jugadores:{}", StrColors::YELLOW, StrColors::RESET);
        const auto& player0 = gameState.getPlayer(0);
        const auto& player1 = gameState.getPlayer(1);
        
        std::println("Jugador 0 vivo: {}", player0.isAlive() ? "Sí" : "No");
        std::println("Jugador 1 vivo: {}", player1.isAlive() ? "Sí" : "No");
        
        // Test de victoria por destrucción de leyenda
        std::println("\n{}=== SIMULANDO DESTRUCCIÓN DE LEYENDA ==={}", StrColors::BOLD, StrColors::RESET);
        
        if (spawn1Cell && spawn1Cell->card.has_value()) {
            auto legendToDestroy = spawn1Cell->card.value();
            std::println("Destruyendo leyenda: {}{}{}", StrColors::RED, legendToDestroy->getName(), StrColors::RESET);
            
            gameState.destroyCard(legendToDestroy);
            
            std::println("\n{}Mapa después de destruir leyenda:{}", StrColors::CYAN, StrColors::RESET);
            std::println("{}", (std::string) gameState.getMap());
            
            // Verificar estado del juego
            std::println("\n{}Estado del juego:{}", StrColors::YELLOW, StrColors::RESET);
            std::println("¿Juego terminado? {}", gameState.isGameOver() ? "Sí" : "No");
            
            if (gameState.isGameOver()) {
                auto winner = gameState.getWinner();
                if (winner.has_value()) {
                    std::println("{}¡Ganador: Team {}!{}", StrColors::GREEN, 
                               static_cast<int>(winner.value()), StrColors::RESET);
                } else {
                    std::println("{}¡Empate!{}", StrColors::YELLOW, StrColors::RESET);
                }
            }
        }
        
        std::println("\n{}Test de sistema de leyendas completado!{}", StrColors::GREEN, StrColors::RESET);
        
    } catch (const std::exception& e) {
        std::println("{}Error durante el test: {}{}", StrColors::RED, e.what(), StrColors::RESET);
    }
}

int main(int argc, char** argv) {
    std::println("{0}Card Loading System Test{1}", StrColors::BOLD, StrColors::RESET);
    std::println("========================================");

    try {
        // Load decks from JSON
        std::vector<CardLoader::DeckConfig> deckConfigs = CardLoader::loadDecksFromFile("decks.json");
        
        std::println("Loaded {} deck configurations.", deckConfigs.size());

        // Si se pasa un argumento, determinar qué test ejecutar
        if (argc > 1) {
            std::string testType = argv[1];
            
            if (testType == "basic") {
                // Test básico
                for (const auto& deckConfig : deckConfigs) {
                    std::vector<CardPtr> deck = CardLoader::createCardsFromConfig(deckConfig, 0);
                    printDeck(deck, deckConfig.name);
                }
                testGameWithLoadedDecks();
            }
            else if (testType == "advanced") {
                // Test avanzado con secuencia de acciones
                testAdvancedGameSequence();
            }
            else if (testType == "legends") {
                // Test prioritario del sistema de leyendas
                testLegendSystemAndSpawns();
            }
            else {
                std::println("Tipo de test no reconocido. Opciones: basic, advanced, legends");
            }
        }
        else {
            // Por defecto, ejecutar test de leyendas primero (prioritario)
            testLegendSystemAndSpawns();
            
            std::println("\n\n");
            std::println("========================================");
            
            for (const auto& deckConfig : deckConfigs) {
                std::vector<CardPtr> deck = CardLoader::createCardsFromConfig(deckConfig, 0);
                printDeck(deck, deckConfig.name);
            }
            testGameWithLoadedDecks();
            
            std::println("\n\n");
            std::println("========================================");
            
            testAdvancedGameSequence();
        }

    } catch (const std::exception& e) {
        std::println("Error: {}", e.what());
        return 1;
    }

    return 0;
}
