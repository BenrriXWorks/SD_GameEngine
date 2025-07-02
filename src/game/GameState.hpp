#pragma once
#include <array>
#include <cstdint>
#include <utility>
#include <vector>
#include <list>
#include <algorithm>
#include "GameMap.hpp"
#include "../effects/EffectStack.hpp"
#include "../utils/Types.hpp"
#include "../cards/Card.hpp"

enum class Team : uint8_t {
    NONE = 0,
    TEAM_A = 1,
    TEAM_B = 2
};

enum class GamePhase : uint8_t {
    SETUP = 0,
    DEPLOY = 1,
    PLAY = 2,
    END = 3
};

struct Player {
    PlayerId id;
    Team team;
    std::string name;
    uint8_t health = 20;
    uint8_t actionsRemaining = 3;  // Acciones por turno
    uint8_t maxActionsPerTurn = 3; // Configurable
    
    std::vector<CardPtr> deck;
    std::vector<CardPtr> hand;
    std::vector<CardPtr> discard;
    
    // Referencia a la leyenda del jugador (nullptr si no tiene o murió)
    std::shared_ptr<Legend> legend = nullptr;
    
    Player(PlayerId playerId, Team playerTeam, const std::string& playerName)
        : id(playerId), team(playerTeam), name(playerName) {}
        
    // Helper methods for hand management
    size_t findCardIndex(CardPtr card) const;
    bool hasCard(CardPtr card) const;
    
    // Helper para verificar si el jugador sigue vivo (leyenda viva)
    bool isAlive() const { return legend && legend->isAlive(); }
};

// Structure to represent an action in the game
struct GameAction {
    PlayerId playerId;
    CardPtr card;
    uint8_t x = 0, y = 0;
    
    enum class ActionType : uint8_t {
        PLAY_CARD,
        MOVE_CARD,
        ATTACK,
        END_TURN
    } type;
    
    // Target for attacks or targeted spells
    std::optional<std::pair<uint8_t, uint8_t>> target;
};

// Main game state class
class GameState {
private:
    GameMap map;
    EffectStack effectStack;
    std::vector<Player> players;
    
    PlayerId currentPlayer = 0;
    uint32_t turnNumber = 0;
    GamePhase phase = GamePhase::SETUP;
    
    // Lightweight thread-local RNG for better performance with multiple games
    // Using a simple but fast LCG (Linear Congruential Generator)
    static thread_local uint32_t rng_state;
    
    // Internal helper methods
    void drawCard(Player& player, uint8_t count = 1);
    
    // Método para devolver una carta al mazo del jugador y barajarlo
    void returnCardToDeck(PlayerId playerId, CardPtr card);
    
    // Simple helper methods (no cache needed for few players)
    Player* findPlayer(PlayerId id);
    const Player* findPlayer(PlayerId id) const;
    bool hasActionsRemaining(PlayerId playerId) const;
    bool consumeAction(PlayerId playerId);
    void resetPlayerActions(PlayerId playerId);
    
    // Fast thread-local random number generation
    static uint32_t fastRand();
    static void seedRng(uint32_t seed = 0);
    template<typename Container>
    static void shuffleContainer(Container& container);
    
public:
    GameState();
    
    // Game setup
    void addPlayer(PlayerId id, Team team, const std::string& name);
    void setPlayerDeck(PlayerId id, std::vector<CardPtr> deck);
    
    // Game flow control
    void startGame();
    void endTurn(PlayerId playerId);
    
    // Legend management
    void placeLegends();
    void checkLegendStatus();
    std::shared_ptr<Legend> findLegendInDeck(std::vector<CardPtr>& deck);
    
    // Action processing
    bool processAction(const GameAction& action);
    
    // Card and map operations
    void playCard(PlayerId playerId, CardPtr card, uint8_t x, uint8_t y);
    bool moveCard(PlayerId playerId, CardPtr card, uint8_t x, uint8_t y);
    bool attackWithCard(PlayerId playerId, CardPtr card, uint8_t targetX, uint8_t targetY);
    
    // Combat system
    bool canAttack(PlayerId playerId, CardPtr attacker, uint8_t targetX, uint8_t targetY) const;
    bool canMoveCard(PlayerId playerId, CardPtr card, uint8_t fromX, uint8_t fromY, uint8_t toX, uint8_t toY) const;
    void dealDamage(PlayerId targetPlayer, uint8_t damage);
    void destroyCard(CardPtr card);
    
    // Position validation helpers
    bool isValidPosition(uint8_t x, uint8_t y) const;
    bool isPositionEmpty(uint8_t x, uint8_t y) const;
    bool isPlayerAlive(PlayerId playerId) const;
    
    // State queries
    GamePhase getPhase() const { return phase; }
    PlayerId getCurrentPlayer() const { return currentPlayer; }
    uint32_t getTurnNumber() const { return turnNumber; }
    const GameMap& getMap() const { return map; }
    GameMap& getMap() { return map; }
    const Player& getPlayer(PlayerId id) const;
    
    // Action system queries
    uint8_t getActionsRemaining(PlayerId playerId) const;
    uint8_t getMaxActionsPerTurn(PlayerId playerId) const;
    void setMaxActionsPerTurn(PlayerId playerId, uint8_t maxActions);
    
    // RNG utilities (for testing and configuration)
    static void setSeed(uint32_t seed) { seedRng(seed); }
    static uint32_t getRandom() { return fastRand(); }
    
    // Game state validation and win conditions
    bool isGameOver() const;
    std::optional<Team> getWinner() const;
    
    // Método para transformar dirección basado en el ID del jugador
    GameMap::Adjacency transformDirectionForPlayer(PlayerId playerId, GameMap::Adjacency direction) const {
        // Solo invertir para el jugador 1
        if (playerId == 1) {
            return GameMap::invertDirection(direction);
        }
        return direction;
    }
};

// Serialization operators
inline std::string to_string(const GameState& gameState) {
    std::string result = "Game State:\n";
    result += "Turn: " + std::to_string(gameState.getTurnNumber()) + "\n";
    result += "Current Player: " + std::to_string(gameState.getCurrentPlayer()) + "\n";
    result += "Phase: ";
    
    switch (gameState.getPhase()) {
        case GamePhase::SETUP: result += "Setup\n"; break;
        case GamePhase::DEPLOY: result += "Deploy\n"; break;
        case GamePhase::PLAY: result += "Play\n"; break;
        case GamePhase::END: result += "End\n"; break;
    }
    
    result += "Map:\n";
    result += std::string(gameState.getMap());
    
    return result;
}

// Game setup factory methods
inline GameState createNewGame(uint8_t numPlayers = 2) {
    GameState gameState;
    
    for (uint8_t i = 0; i < numPlayers; ++i) {
        PlayerId id = i + 1;
        Team team = (i & 1) ? Team::TEAM_B : Team::TEAM_A;
        std::string name = "Player " + std::to_string(id);
        
        gameState.addPlayer(id, team, name);
    }
    
    return gameState;
}

// Convenience types
using GameStatePtr = std::shared_ptr<GameState>;
