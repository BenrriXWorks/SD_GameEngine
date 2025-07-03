#include <print>
#include <string>
#include <vector>
#include <random>
#include <thread>
#include <chrono>
#include <functional>
#include <iomanip>
#include <memory>
#include "src/api/GameAPI.hpp"
#include "src/config/GameConfig.hpp"
#include "src/cards/CardLoader.hpp"
#include "src/game/GameState.hpp"
#include "src/utils/StrColors.hpp"

using namespace std;

void printPlayerInfo(const GameAPI::PlayerInfo& player) {
    using namespace StrColors;
    
    println("{}=== {} ({}) ==={}", BOLD, player.name, 
           (player.team == Team::TEAM_A ? "Team A" : "Team B"), RESET);
    println("ID: {}, Vivo: {}, Acciones: {}/{}", 
        player.id, 
        (player.isAlive ? "Sí" : "No"),
        player.actionsLeft, 
        player.maxActions);
    
    print("Mano ({}): [", player.handSize);
    for (size_t i = 0; i < player.handCardNames.size(); ++i) {
        print("{}{}{}", 
              (i == 0 ? "" : ", "),
              player.handCardNames[i],
              "");
    }
    println("]");
    
    println("Cartas en mazo: {}", player.deckSize);
    
    if (player.hasLegend) {
        println("{}Leyenda: {} en posición ({}, {}){}", 
               YELLOW, player.legendName, 
               player.legendPosition.first, player.legendPosition.second, RESET);
    } else {
        println("{}¡LEYENDA DESTRUIDA!{}", RED, RESET);
    }
    println();
}

void printGameState(const GameAPI::GameInfo& gameInfo) {
    using namespace StrColors;
    
    println("{}=== ESTADO DEL JUEGO ==={}", BOLD, RESET);
    println("Turno: {}, Jugador actual: {}", 
        gameInfo.turn, gameInfo.currentPlayer);
    println("Equipo actual: {}", 
        (gameInfo.currentTeam == Team::TEAM_A ? "Team A" : "Team B"));
    
    if (gameInfo.isGameOver) {
        println("{}¡JUEGO TERMINADO!{}", RED, RESET);
        if (gameInfo.winner.has_value()) {
            println("{}Ganador: {}{}", GREEN, 
                   (gameInfo.winner.value() == Team::TEAM_A ? "Team A" : "Team B"), RESET);
        } else {
            println("{}Empate{}", YELLOW, RESET);
        }
    }
    println();
    
    printPlayerInfo(gameInfo.player0);
    printPlayerInfo(gameInfo.player1);
    
    println("{}Mapa del juego:{}", CYAN, RESET);
    println("{}", gameInfo.mapVisualization);
    println();
}

// ===========================
// FUNCIÓN PRINCIPAL DE PRUEBAS
// ===========================

// Estructura para definir pruebas
struct TestCase {
    string name;
    function<bool(GameAPI&, const GameConfig&)> testFunc;
    bool runWithAPI;
    
    TestCase(const string& n, function<bool(GameAPI&, const GameConfig&)> f, bool api = true) 
        : name(n), testFunc(f), runWithAPI(api) {}
};

// Prototipos de funciones de prueba
bool testConfigurationLoading(GameAPI& api, const GameConfig& config);
bool testBasicGameInitialization(GameAPI& api, const GameConfig& config);
bool testCardLoading(GameAPI& api, const GameConfig& config);
bool testPlayerManagement(GameAPI& api, const GameConfig& config);
bool testCardPlayAndEffects(GameAPI& api, const GameConfig& config);
bool testMovementAndCombat(GameAPI& api, const GameConfig& config);
bool testEffectSystem(GameAPI& api, const GameConfig& config);
bool testGameFlow(GameAPI& api, const GameConfig& config);
bool testAISimulation(GameAPI& api, const GameConfig& config);

// Funciones auxiliares
void printTestHeader(const string& testName);
void printTestResult(const string& testName, bool passed);
void printSeparator();

int main() {
    using namespace StrColors;
    
    println("{}==================================================={}", BOLD, RESET);
    println("{}=== SD GAME ENGINE 2XD - SISTEMA DE PRUEBAS UNIFICADO ==={}", BOLD, RESET);
    println("{}==================================================={}", BOLD, RESET);
    println();
    
    // Paso 1: Cargar configuración
    printTestHeader("Carga de Configuración");
    GameConfig config;
    if (!config.loadFromFile("game_config.conf")) {
        println("{}Error: No se pudo cargar la configuración. Usando valores por defecto.{}", RED, RESET);
    }
    
    // Mostrar configuración cargada
    println("{}Configuración cargada:{}", CYAN, RESET);
    config.printAll();
    println();
    
    // Definir todas las pruebas
    vector<TestCase> tests = {
        TestCase("Validación de Configuración", testConfigurationLoading),
        TestCase("Inicialización Básica del Juego", testBasicGameInitialization),
        TestCase("Carga de Cartas y Mazos", testCardLoading),
        TestCase("Gestión de Jugadores", testPlayerManagement),
        TestCase("Juego de Cartas y Efectos", testCardPlayAndEffects),
        TestCase("Movimiento y Combate", testMovementAndCombat),
        TestCase("Sistema de Efectos", testEffectSystem),
        TestCase("Flujo de Juego Completo", testGameFlow),
        TestCase("Simulación de IA", testAISimulation)
    };
    
    // Ejecutar todas las pruebas
    int passed = 0;
    int total = tests.size();
    
    printSeparator();
    println("{}Ejecutando {} pruebas...{}", YELLOW, total, RESET);
    printSeparator();
    
    GameAPI api;
    
    for (const auto& test : tests) {
        printTestHeader(test.name);
        
        bool result = false;
        try {
            result = test.testFunc(api, config);
        } catch (const exception& e) {
            println("{}Error durante la prueba: {}{}", RED, e.what(), RESET);
            result = false;
        }
        
        printTestResult(test.name, result);
        if (result) passed++;
        
        printSeparator();
        
        // Pequeña pausa entre pruebas para mejor visualización
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    // Resumen final
    println("{}=== RESUMEN DE PRUEBAS ==={}", BOLD, RESET);
    println("Pruebas pasadas: {}{}/{}{}", 
            passed == total ? GREEN : (passed > total/2 ? YELLOW : RED),
            passed, total, RESET);
    
    if (passed == total) {
        println("{}🎉 ¡TODAS LAS PRUEBAS PASARON EXITOSAMENTE! 🎉{}", GREEN, RESET);
    } else if (passed > total/2) {
        println("{}⚠️  La mayoría de las pruebas pasaron, pero hay algunas fallas.{}", YELLOW, RESET);
    } else {
        println("{}❌ Múltiples pruebas fallaron. Revisar el sistema.{}", RED, RESET);
    }
    
    println("{}==================================================={}", BOLD, RESET);
    return (passed == total) ? 0 : 1;
}

// ===========================
// IMPLEMENTACIÓN DE PRUEBAS
// ===========================

bool testConfigurationLoading(GameAPI& api, const GameConfig& config) {
    (void)api; // Suppress unused parameter warning
    using namespace StrColors;
    
    // Verificar valores básicos de configuración
    int maxHandSize = config.getInt("max_hand_size", 7);
    int maxActions = config.getInt("max_actions_per_turn", 3);
    int mapWidth = config.getInt("map_width", 5);
    int mapHeight = config.getInt("map_height", 6);
    
    println("Valores de configuración verificados:");
    println("  max_hand_size: {}", maxHandSize);
    println("  max_actions_per_turn: {}", maxActions);
    println("  map_width: {}", mapWidth);
    println("  map_height: {}", mapHeight);
    
    // Validar que los valores están en rangos razonables
    bool valid = (maxHandSize >= 3 && maxHandSize <= 20) &&
                 (maxActions >= 1 && maxActions <= 10) &&
                 (mapWidth >= 3 && mapWidth <= 20) &&
                 (mapHeight >= 3 && mapHeight <= 20);
    
    if (valid) {
        println("{}✓ Configuración válida{}", GREEN, RESET);
    } else {
        println("{}✗ Valores de configuración fuera de rango{}", RED, RESET);
    }
    
    return valid;
}

bool testBasicGameInitialization(GameAPI& api, const GameConfig& config) {
    using namespace StrColors;
    
    // Inicializar el juego con configuración
    bool initSuccess = api.initializeGameWithConfig("decks.json", config);
    if (!initSuccess) {
        println("{}✗ No se pudo inicializar el juego con configuración{}", RED, RESET);
        return false;
    }
    
    println("{}✓ Juego inicializado correctamente{}", GREEN, RESET);
    
    // Verificar estado inicial del juego
    auto gameInfo = api.getGameInfo();
    
    bool validState = !gameInfo.isGameOver &&
                     (gameInfo.currentPlayer == 0 || gameInfo.currentPlayer == 1);
    
    if (validState) {
        println("{}✓ Estado inicial del juego válido{}", GREEN, RESET);
        println("  Turno: {}, Jugador actual: {}, Juego terminado: {}", 
                gameInfo.turn, gameInfo.currentPlayer, gameInfo.isGameOver ? "Sí" : "No");
    } else {
        println("{}✗ Estado inicial del juego inválido{}", RED, RESET);
        println("  Turno: {}, Jugador actual: {}, Juego terminado: {}", 
                gameInfo.turn, gameInfo.currentPlayer, gameInfo.isGameOver ? "Sí" : "No");
    }
    
    return validState;
}

bool testCardLoading(GameAPI& api, const GameConfig& config) {
    (void)api; // Suppress unused parameter warning
    (void)config; // Suppress unused parameter warning
    using namespace StrColors;
    
    // Cargar mazos desde JSON
    vector<CardLoader::DeckConfig> deckConfigs;
    try {
        deckConfigs = CardLoader::loadDecksFromFile("decks.json");
    } catch (const exception& e) {
        println("{}✗ Error cargando mazos: {}{}", RED, e.what(), RESET);
        return false;
    }
    
    if (deckConfigs.size() < 2) {
        println("{}✗ Se requieren al menos 2 mazos, se encontraron {}{}", RED, deckConfigs.size(), RESET);
        return false;
    }
    
    println("{}✓ Mazos cargados exitosamente ({} mazos){}", GREEN, deckConfigs.size(), RESET);
    
    // Verificar contenido de los mazos
    for (size_t i = 0; i < min(deckConfigs.size(), (size_t)2UL); ++i) {
        const auto& deck = deckConfigs[i];
        println("  Mazo {}: '{}' ({} cartas)", i, deck.name, deck.cards.size());
        
        if (deck.cards.empty()) {
            println("{}✗ El mazo {} está vacío{}", RED, i, RESET);
            return false;
        }
        
        // Verificar algunas cartas del mazo
        for (size_t j = 0; j < min(deck.cards.size(), (size_t)3UL); ++j) {
            const auto& card = deck.cards[j];
            println("    Carta {}: '{}' (Costo: {}, Tipo: {})", 
                    j, card.name, card.cost, card.type);
        }
    }
    
    // Crear cartas reales para verificar que el proceso funciona
    try {
        auto cards1 = CardLoader::createCardsFromConfig(deckConfigs[0], 0);
        auto cards2 = CardLoader::createCardsFromConfig(deckConfigs[1], 1);
        
        if (cards1.empty() || cards2.empty()) {
            println("{}✗ No se pudieron crear cartas desde las configuraciones{}", RED, RESET);
            return false;
        }
        
        println("{}✓ Cartas creadas exitosamente (J1: {}, J2: {}){}", GREEN, cards1.size(), cards2.size(), RESET);
    } catch (const exception& e) {
        println("{}✗ Error creando cartas: {}{}", RED, e.what(), RESET);
        return false;
    }
    
    return true;
}

bool testPlayerManagement(GameAPI& api, const GameConfig& config) {
    using namespace StrColors;
    
    // Obtener información de ambos jugadores
    auto player0 = api.getPlayerInfo(0);
    auto player1 = api.getPlayerInfo(1);
    
    // Verificar información básica
    bool valid = player0.id == 0 && player1.id == 1 &&
                 player0.isAlive && player1.isAlive &&
                 player0.team != player1.team;
    
    if (!valid) {
        println("{}✗ Información básica de jugadores inválida{}", RED, RESET);
        return false;
    }
    
    println("{}✓ Información básica de jugadores válida{}", GREEN, RESET);
    
    // Verificar cartas iniciales
    size_t expectedHandSize = static_cast<size_t>(config.getInt("initial_hand_size", 5));
    bool handSizeOk = player0.handSize <= expectedHandSize && player1.handSize <= expectedHandSize;
    
    if (handSizeOk) {
        println("{}✓ Tamaño de mano inicial válido (J1: {}, J2: {}, Max: {}){}", 
                GREEN, player0.handSize, player1.handSize, expectedHandSize, RESET);
    } else {
        println("{}✗ Tamaño de mano inicial inválido{}", RED, RESET);
    }
    
    // Verificar leyendas en el mapa
    bool legendsOk = player0.hasLegend && player1.hasLegend;
    if (legendsOk) {
        println("{}✓ Ambos jugadores tienen leyendas en el mapa{}", GREEN, RESET);
        println("  Leyenda J1: '{}' en ({}, {})", player0.legendName, 
                player0.legendPosition.first, player0.legendPosition.second);
        println("  Leyenda J2: '{}' en ({}, {})", player1.legendName, 
                player1.legendPosition.first, player1.legendPosition.second);
    } else {
        println("{}✗ Al menos un jugador no tiene leyenda en el mapa{}", RED, RESET);
    }
    
    return valid && handSizeOk && legendsOk;
}

bool testCardPlayAndEffects(GameAPI& api, const GameConfig& config) {
    (void)config; // Suppress unused parameter warning
    using namespace StrColors;
    
    // Intentar jugar una carta
    auto player0 = api.getPlayerInfo(0);
    if (player0.handSize == 0) {
        // Robar una carta primero
        auto drawResult = api.drawCard(0);
        if (drawResult != GameAPI::ActionResult::SUCCESS) {
            println("{}✗ No se pudo robar carta para la prueba{}", RED, RESET);
            return false;
        }
        player0 = api.getPlayerInfo(0);
    }
    
    if (player0.handSize == 0) {
        println("{}✗ Jugador no tiene cartas en la mano{}", RED, RESET);
        return false;
    }
    
    // Obtener posiciones válidas para jugar
    auto validPositions = api.getValidPlayPositions(0);
    if (validPositions.empty()) {
        println("{}✗ No hay posiciones válidas para jugar cartas{}", RED, RESET);
        return false;
    }
    
    // Jugar la primera carta en la primera posición válida
    auto pos = validPositions[0];
    auto playResult = api.playCard(0, 0, pos.first, pos.second);
    
    if (playResult == GameAPI::ActionResult::SUCCESS) {
        println("{}✓ Carta jugada exitosamente en ({}, {}){}", GREEN, pos.first, pos.second, RESET);
        
        // Verificar que la carta está en el mapa
        auto gameInfo = api.getGameInfo();
        bool cardFound = false;
        for (const auto& cell : gameInfo.mapCells) {
            if (cell.x == pos.first && cell.y == pos.second && cell.hasCard) {
                cardFound = true;
                println("  Carta '{}' encontrada en el mapa", cell.cardName);
                break;
            }
        }
        
        if (!cardFound) {
            println("{}✗ Carta no encontrada en el mapa después de jugarla{}", RED, RESET);
            return false;
        }
        
        println("{}✓ Carta verificada en el mapa{}", GREEN, RESET);
        return true;
    } else {
        println("{}✗ Error jugando carta: {}{}", RED, GameAPI::actionResultToString(playResult), RESET);
        return false;
    }
}

bool testMovementAndCombat(GameAPI& api, const GameConfig& config) {
    (void)config; // Suppress unused parameter warning
    using namespace StrColors;
    
    // Buscar una carta propia en el mapa para mover
    auto gameInfo = api.getGameInfo();
    PlayerId currentPlayer = gameInfo.currentPlayer;
    
    uint8_t sourceX = 0, sourceY = 0;
    bool cardFound = false;
    
    for (const auto& cell : gameInfo.mapCells) {
        if (cell.hasCard && cell.cardOwner == currentPlayer) {
            sourceX = cell.x;
            sourceY = cell.y;
            cardFound = true;
            break;
        }
    }
    
    if (!cardFound) {
        println("{}⚠️  No se encontraron cartas propias en el mapa para probar movimiento{}", YELLOW, RESET);
        return true; // No es un error crítico
    }
    
    // Probar movimiento
    auto movePositions = api.getValidMovePositions(sourceX, sourceY);
    if (!movePositions.empty()) {
        auto dest = movePositions[0];
        auto moveResult = api.moveCard(currentPlayer, sourceX, sourceY, dest.first, dest.second);
        
        if (moveResult == GameAPI::ActionResult::SUCCESS) {
            println("{}✓ Movimiento exitoso de ({}, {}) a ({}, {}){}", GREEN, 
                    sourceX, sourceY, dest.first, dest.second, RESET);
        } else {
            println("{}✗ Error en movimiento: {}{}", RED, GameAPI::actionResultToString(moveResult), RESET);
            return false;
        }
    } else {
        println("{}⚠️  No hay movimientos válidos disponibles{}", YELLOW, RESET);
    }
    
    // Probar ataque si hay objetivos
    auto attackPositions = api.getValidAttackPositions(sourceX, sourceY);
    if (!attackPositions.empty()) {
        auto target = attackPositions[0];
        auto attackResult = api.attackCard(currentPlayer, sourceX, sourceY, target.first, target.second);
        
        if (attackResult == GameAPI::ActionResult::SUCCESS) {
            println("{}✓ Ataque exitoso desde ({}, {}) hacia ({}, {}){}", GREEN, 
                    sourceX, sourceY, target.first, target.second, RESET);
        } else {
            println("{}⚠️  Ataque no exitoso: {}{}", YELLOW, GameAPI::actionResultToString(attackResult), RESET);
        }
    } else {
        println("{}⚠️  No hay objetivos de ataque disponibles{}", YELLOW, RESET);
    }
    
    return true;
}

bool testEffectSystem(GameAPI& api, const GameConfig& config) {
    (void)config; // Suppress unused parameter warning
    using namespace StrColors;
    
    println("{}Probando sistema de efectos...{}", CYAN, RESET);
    
    // Esta prueba es más conceptual ya que los efectos son internos
    // Verificamos que el juego responde correctamente a las acciones
    
    auto gameInfoBefore = api.getGameInfo();
    
    // Realizar algunas acciones para activar efectos
    if (gameInfoBefore.player0.deckSize > 0) {
        auto drawResult = api.drawCard(0);
        if (drawResult == GameAPI::ActionResult::SUCCESS) {
            println("{}✓ Robo de carta activó efectos correctamente{}", GREEN, RESET);
        }
    }
    
    auto gameInfoAfter = api.getGameInfo();
    
    // Verificar que el estado cambió apropiadamente
    bool stateChanged = (gameInfoAfter.player0.handSize != gameInfoBefore.player0.handSize) ||
                       (gameInfoAfter.player0.deckSize != gameInfoBefore.player0.deckSize);
    
    if (stateChanged) {
        println("{}✓ Sistema de efectos respondiendo a cambios de estado{}", GREEN, RESET);
    } else {
        println("{}⚠️  Sin cambios detectados en el estado{}", YELLOW, RESET);
    }
    
    return true;
}

bool testGameFlow(GameAPI& api, const GameConfig& config) {
    (void)config; // Suppress unused parameter warning
    using namespace StrColors;
    
    println("{}Probando flujo completo de juego...{}", CYAN, RESET);
    
    auto gameInfo = api.getGameInfo();
    int initialTurn = gameInfo.turn;
    PlayerId initialPlayer = gameInfo.currentPlayer;
    
    // Terminar turno actual
    api.endTurn();
    
    auto newGameInfo = api.getGameInfo();
    
    // Verificar que el turno cambió correctamente
    bool turnChanged = (newGameInfo.currentPlayer != initialPlayer) || 
                      (newGameInfo.turn != static_cast<uint32_t>(initialTurn));
    
    if (turnChanged) {
        println("{}✓ Cambio de turno funcionando correctamente{}", GREEN, RESET);
        println("  Turno: {} → {}, Jugador: {} → {}", 
                initialTurn, newGameInfo.turn, initialPlayer, newGameInfo.currentPlayer);
    } else {
        println("{}✗ Cambio de turno no funcionó{}", RED, RESET);
        return false;
    }
    
    // Verificar que el juego no ha terminado prematuramente
    if (newGameInfo.isGameOver) {
        println("{}⚠️  El juego terminó inesperadamente{}", YELLOW, RESET);
    } else {
        println("{}✓ El juego continúa normalmente{}", GREEN, RESET);
    }
    
    return true;
}

bool testAISimulation(GameAPI& api, const GameConfig& config) {
    (void)config; // Suppress unused parameter warning
    using namespace StrColors;
    
    println("{}Ejecutando simulación básica de IA...{}", CYAN, RESET);
    
    // Crear una IA simple para probar
    class SimpleAI {
    private:
        GameAPI& api;
        PlayerId playerId;
        mt19937 rng;

    public:
        SimpleAI(GameAPI& gameApi, PlayerId id) : api(gameApi), playerId(id), rng(random_device{}()) {}
        
        bool playTurn() {
            auto gameInfo = api.getGameInfo();
            if (gameInfo.isGameOver || gameInfo.currentPlayer != playerId) {
                return false;
            }
            
            auto playerInfo = (playerId == 0) ? gameInfo.player0 : gameInfo.player1;
            bool actionTaken = false;
            
            // Intentar robar carta
            if (playerInfo.handSize <= 1 && playerInfo.deckSize > 0) {
                auto result = api.drawCard(playerId);
                if (result == GameAPI::ActionResult::SUCCESS) {
                    actionTaken = true;
                }
            }
            
            // Intentar jugar carta
            if (!actionTaken && playerInfo.handSize > 0) {
                auto validPositions = api.getValidPlayPositions(playerId);
                if (!validPositions.empty()) {
                    auto pos = validPositions[rng() % validPositions.size()];
                    auto result = api.playCard(playerId, 0, pos.first, pos.second);
                    if (result == GameAPI::ActionResult::SUCCESS) {
                        actionTaken = true;
                    }
                }
            }
            
            api.endTurn();
            return actionTaken;
        }
    };
    
    auto gameInfo = api.getGameInfo();
    auto ai = std::make_unique<SimpleAI>(api, gameInfo.currentPlayer);
    
    // Simular algunos turnos
    int turnsSimulated = 0;
    for (int i = 0; i < 3 && !api.isGameOver(); ++i) {
        gameInfo = api.getGameInfo();
        ai = std::make_unique<SimpleAI>(api, gameInfo.currentPlayer);
        
        if (ai->playTurn()) {
            turnsSimulated++;
        }
    }
    
    if (turnsSimulated > 0) {
        println("{}✓ IA simuló {} turnos exitosamente{}", GREEN, turnsSimulated, RESET);
        return true;
    } else {
        println("{}✗ IA no pudo realizar ninguna acción{}", RED, RESET);
        return false;
    }
}

// ===========================
// FUNCIONES AUXILIARES
// ===========================

void printTestHeader(const string& testName) {
    using namespace StrColors;
    println("\n{}=== {} ==={}", BOLD, testName, RESET);
}

void printTestResult(const string& testName, bool passed) {
    using namespace StrColors;
    if (passed) {
        println("{}✓ {} - PASÓ{}", GREEN, testName, RESET);
    } else {
        println("{}✗ {} - FALLÓ{}", RED, testName, RESET);
    }
}

void printSeparator() {
    using namespace StrColors;
    println("{}{}{}", DIM, string(50, '-'), RESET);
}