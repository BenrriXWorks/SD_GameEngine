#pragma once
#include <vector>
#include <string>
#include <optional>
#include <memory>
#include "../cards/Card.hpp"
#include "../game/GameState.hpp"
#include "../cards/CardLoader.hpp"
#include "../config/GameConfig.hpp"
#include "../utils/Types.hpp"

/**
 * API completa para el juego de cartas
 * Esta clase encapsula toda la funcionalidad necesaria para una partida completa
 */
class GameAPI {
public:
    // Estructura para representar el estado de un jugador
    struct PlayerInfo {
        PlayerId id;
        Team team;
        std::string name;
        bool isAlive;
        uint8_t actionsLeft;
        uint8_t maxActions;
        size_t handSize;
        size_t deckSize;
        std::vector<std::string> handCardNames;
        bool hasLegend;
        std::string legendName;
        std::pair<uint8_t, uint8_t> legendPosition;
    };

    // Estructura para representar información de una celda del mapa
    struct CellInfo {
        uint8_t x, y;
        bool isWalkable;
        bool isSpawn;
        bool hasCard;
        std::string cardName;
        std::string cardType;
        PlayerId cardOwner;
        uint8_t attack;
        uint8_t health;
        uint8_t speed;
        uint8_t range;
        bool isLegend;
    };

    // Estructura para el estado completo del juego
    struct GameInfo {
        uint32_t turn;
        Team currentTeam;
        PlayerId currentPlayer;
        bool isGameOver;
        std::optional<Team> winner;
        PlayerInfo player0;
        PlayerInfo player1;
        std::vector<CellInfo> mapCells;
        std::string mapVisualization;
    };

    // Resultado de una acción
    enum class ActionResult {
        SUCCESS,
        INVALID_PLAYER,
        INVALID_POSITION,
        INVALID_CARD,
        NOT_ENOUGH_ACTIONS,
        POSITION_OCCUPIED,
        CARD_NOT_IN_HAND,
        CANNOT_ATTACK_TARGET,
        OUT_OF_RANGE,
        GAME_OVER,
        INVALID_MOVE
    };

private:
    std::unique_ptr<GameState> gameState;
    bool initialized;

public:
    // Constructor
    GameAPI();
    ~GameAPI() = default;

    // === INICIALIZACIÓN DEL JUEGO ===
    
    /**
     * Inicializa una nueva partida con mazos desde archivo JSON
     * @param deckFilePath Ruta al archivo JSON con los mazos
     * @param player0Name Nombre del jugador 0
     * @param player1Name Nombre del jugador 1
     * @return true si la inicialización fue exitosa
     */
    bool initializeGame(const std::string& deckFilePath, 
                       const std::string& player0Name = "Player 0", 
                       const std::string& player1Name = "Player 1");

    /**
     * Inicializa una nueva partida con mazos personalizados
     * @param deck0 Mazo del jugador 0
     * @param deck1 Mazo del jugador 1
     * @param player0Name Nombre del jugador 0
     * @param player1Name Nombre del jugador 1
     * @return true si la inicialización fue exitosa
     */
    bool initializeGame(const std::vector<CardPtr>& deck0, const std::vector<CardPtr>& deck1,
                       const std::string& player0Name = "Player 0", 
                       const std::string& player1Name = "Player 1");

    /**
     * Inicializa una nueva partida con archivo de mazos y configuración personalizada
     * @param deckFilePath Ruta al archivo JSON con los mazos
     * @param player0Name Nombre del jugador 0
     * @param player1Name Nombre del jugador 1
     * @param config Configuración del juego
     * @return true si la inicialización fue exitosa
     */
    bool initializeGameWithConfig(const std::string& deckFilePath, 
                                 const GameConfig& config);

    // === INFORMACIÓN DEL JUEGO ===
    
    /**
     * Obtiene el estado completo del juego
     */
    GameInfo getGameInfo() const;

    /**
     * Obtiene información de un jugador específico
     */
    PlayerInfo getPlayerInfo(PlayerId playerId) const;

    /**
     * Obtiene información de una celda del mapa
     */
    std::optional<CellInfo> getCellInfo(uint8_t x, uint8_t y) const;

    /**
     * Obtiene la visualización del mapa como string
     */
    std::string getMapVisualization() const;

    /**
     * Verifica si el juego ha terminado
     */
    bool isGameOver() const;

    /**
     * Obtiene el ganador del juego (si hay uno)
     */
    std::optional<Team> getWinner() const;

    // === ACCIONES DE JUEGO ===
    
    /**
     * Jugar una carta desde la mano a una posición
     * @param playerId ID del jugador
     * @param cardIndex Índice de la carta en la mano
     * @param x Coordenada X del destino
     * @param y Coordenada Y del destino
     * @return Resultado de la acción
     */
    ActionResult playCard(PlayerId playerId, size_t cardIndex, uint8_t x, uint8_t y);

    /**
     * Mover una carta en el mapa
     * @param playerId ID del jugador
     * @param fromX Coordenada X origen
     * @param fromY Coordenada Y origen
     * @param toX Coordenada X destino
     * @param toY Coordenada Y destino
     * @return Resultado de la acción
     */
    ActionResult moveCard(PlayerId playerId, uint8_t fromX, uint8_t fromY, uint8_t toX, uint8_t toY);

    /**
     * Atacar con una carta a otra posición
     * @param playerId ID del jugador
     * @param fromX Coordenada X del atacante
     * @param fromY Coordenada Y del atacante
     * @param toX Coordenada X del objetivo
     * @param toY Coordenada Y del objetivo
     * @return Resultado de la acción
     */
    ActionResult attackCard(PlayerId playerId, uint8_t fromX, uint8_t fromY, uint8_t toX, uint8_t toY);

    /**
     * Lanzar un hechizo desde la mano
     * @param playerId ID del jugador
     * @param cardIndex Índice del hechizo en la mano
     * @return Resultado de la acción
     */
    ActionResult castSpell(PlayerId playerId, size_t cardIndex);

    /**
     * Robar una carta del mazo
     * @param playerId ID del jugador
     * @return Resultado de la acción
     */
    ActionResult drawCard(PlayerId playerId);

    /**
     * Terminar el turno del jugador actual
     * @return Resultado de la acción
     */
    ActionResult endTurn();

    // === FUNCIONES DE UTILIDAD ===
    
    /**
     * Obtiene las posiciones válidas donde un jugador puede colocar cartas
     * @param playerId ID del jugador
     * @return Vector de posiciones válidas (x, y)
     */
    std::vector<std::pair<uint8_t, uint8_t>> getValidPlayPositions(PlayerId playerId) const;

    /**
     * Obtiene las posiciones válidas donde una carta puede moverse
     * @param x Coordenada X de la carta
     * @param y Coordenada Y de la carta
     * @return Vector de posiciones válidas (x, y)
     */
    std::vector<std::pair<uint8_t, uint8_t>> getValidMovePositions(uint8_t x, uint8_t y) const;

    /**
     * Obtiene las posiciones válidas que una carta puede atacar
     * @param x Coordenada X de la carta
     * @param y Coordenada Y de la carta
     * @return Vector de posiciones válidas (x, y)
     */
    std::vector<std::pair<uint8_t, uint8_t>> getValidAttackPositions(uint8_t x, uint8_t y) const;

    /**
     * Verifica si una posición es válida para colocar una carta
     * @param playerId ID del jugador
     * @param x Coordenada X
     * @param y Coordenada Y
     * @return true si la posición es válida
     */
    bool isValidPlayPosition(PlayerId playerId, uint8_t x, uint8_t y) const;

    /**
     * Convierte ActionResult a string para debugging
     */
    static std::string actionResultToString(ActionResult result);

private:
    // Funciones auxiliares
    PlayerInfo extractPlayerInfo(const Player& player) const;
    CellInfo extractCellInfo(uint8_t x, uint8_t y) const;
    bool isInitialized() const { return initialized && gameState != nullptr; }
};
