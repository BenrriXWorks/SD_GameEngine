#include "GameAPI.hpp"
#include "../cards/Card.hpp" // Para acceder a Legend
#include <algorithm>
#include <iostream>

GameAPI::GameAPI() : gameState(nullptr), initialized(false) {}

bool GameAPI::initializeGame(const std::string& deckFilePath, 
                           const std::string& player0Name, 
                           const std::string& player1Name) {
    try {
        // Cargar mazos desde archivo JSON
        std::vector<CardLoader::DeckConfig> deckConfigs = CardLoader::loadDecksFromFile(deckFilePath);
        
        if (deckConfigs.size() < 2) {
            return false; // Necesitamos al menos 2 mazos
        }

        // Crear cartas para cada jugador
        std::vector<CardPtr> deck0 = CardLoader::createCardsFromConfig(deckConfigs[0], 0);
        std::vector<CardPtr> deck1 = CardLoader::createCardsFromConfig(deckConfigs[1], 1);

        return initializeGame(deck0, deck1, player0Name, player1Name);
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool GameAPI::initializeGame(const std::vector<CardPtr>& deck0, const std::vector<CardPtr>& deck1,
                           const std::string& player0Name, const std::string& player1Name) {
    try {
        // Crear nuevo estado de juego
        gameState = std::make_unique<GameState>();
        
        // Agregar jugadores
        gameState->addPlayer(0, Team::TEAM_A, player0Name);
        gameState->addPlayer(1, Team::TEAM_B, player1Name);
        
        // Asignar mazos
        gameState->setPlayerDeck(0, deck0);
        gameState->setPlayerDeck(1, deck1);
        
        // Iniciar juego (esto coloca las leyendas automáticamente)
        gameState->startGame();
        
        initialized = true;
        return true;
        
    } catch (const std::exception& e) {
        gameState.reset();
        initialized = false;
        return false;
    }
}

bool GameAPI::initializeGameWithConfig(const std::string& deckFilePath, 
                                     const GameConfig& config) {
    try {
        // Cargar mazos desde archivo JSON
        std::vector<CardLoader::DeckConfig> deckConfigs = CardLoader::loadDecksFromFile(deckFilePath);
        
        if (deckConfigs.size() < 2) {
            return false; // Necesitamos al menos 2 mazos
        }

        // Crear cartas para cada jugador
        std::vector<CardPtr> deck0 = CardLoader::createCardsFromConfig(deckConfigs[0], 0);
        std::vector<CardPtr> deck1 = CardLoader::createCardsFromConfig(deckConfigs[1], 1);

        // Crear nuevo estado de juego con configuración
        gameState = std::make_unique<GameState>(std::move(deck0), std::move(deck1), config);
        
        initialized = true;
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

GameAPI::GameInfo GameAPI::getGameInfo() const {
    if (!isInitialized()) {
        return {}; // Retorna estructura vacía si no está inicializado
    }
    
    GameInfo info;
    info.turn = gameState->getTurnNumber();
    info.currentTeam = (gameState->getCurrentPlayer() == 0) ? Team::TEAM_A : Team::TEAM_B;
    info.currentPlayer = gameState->getCurrentPlayer();
    info.isGameOver = gameState->isGameOver();
    info.winner = gameState->getWinner();
    info.player0 = getPlayerInfo(0);
    info.player1 = getPlayerInfo(1);
    info.mapVisualization = getMapVisualization();
    
    // Llenar información de celdas del mapa
    for (uint8_t y = 0; y < GameMap::MAP_HEIGHT; ++y) {
        for (uint8_t x = 0; x < GameMap::MAP_WIDTH; ++x) {
            info.mapCells.push_back(extractCellInfo(x, y));
        }
    }
    
    return info;
}

GameAPI::PlayerInfo GameAPI::getPlayerInfo(PlayerId playerId) const {
    if (!isInitialized() || playerId > 1) {
        return {}; // Retorna estructura vacía si no es válido
    }
    
    const Player& player = gameState->getPlayer(playerId);
    return extractPlayerInfo(player);
}

std::optional<GameAPI::CellInfo> GameAPI::getCellInfo(uint8_t x, uint8_t y) const {
    if (!isInitialized() || x >= GameMap::MAP_WIDTH || y >= GameMap::MAP_HEIGHT) {
        return std::nullopt;
    }
    
    return extractCellInfo(x, y);
}

std::string GameAPI::getMapVisualization() const {
    if (!isInitialized()) {
        return "";
    }
    
    return std::string(gameState->getMap());
}

bool GameAPI::isGameOver() const {
    if (!isInitialized()) {
        return true;
    }
    
    return gameState->isGameOver();
}

std::optional<Team> GameAPI::getWinner() const {
    if (!isInitialized()) {
        return std::nullopt;
    }
    
    return gameState->getWinner();
}

GameAPI::ActionResult GameAPI::playCard(PlayerId playerId, size_t cardIndex, uint8_t x, uint8_t y) {
    if (!isInitialized()) {
        return ActionResult::GAME_OVER;
    }
    
    if (gameState->isGameOver()) {
        return ActionResult::GAME_OVER;
    }
    
    if (playerId > 1) {
        return ActionResult::INVALID_PLAYER;
    }
    
    const Player& player = gameState->getPlayer(playerId);
    
    // Verificar si es el turno del jugador
    PlayerId currentPlayer = gameState->getCurrentPlayer();
    if (playerId != currentPlayer) {
        return ActionResult::INVALID_PLAYER;
    }
    
    // Verificar acciones disponibles
    if (gameState->getActionsRemaining(playerId) == 0) {
        return ActionResult::NOT_ENOUGH_ACTIONS;
    }
    
    // Verificar índice válido de carta
    if (cardIndex >= player.hand.size()) {
        return ActionResult::INVALID_CARD;
    }
    
    CardPtr card = player.hand[cardIndex];
    if (!card) {
        return ActionResult::INVALID_CARD;
    }
    
    // Verificar que la carta no es una leyenda - las leyendas no se pueden jugar desde la mano
    if (auto legend = std::dynamic_pointer_cast<Legend>(card)) {
        return ActionResult::INVALID_CARD; // Las leyendas no se pueden jugar desde la mano
    }
    
    // Verificar que la carta es una unidad (no se pueden jugar hechizos directamente en el mapa)
    auto unit = std::dynamic_pointer_cast<Unit>(card);
    if (!unit) {
        return ActionResult::INVALID_CARD; // Solo se pueden jugar unidades en el mapa
    }
    
    // Verificar posición válida (incluyendo adyacencia al líder)
    if (!isValidPlayPosition(playerId, x, y)) {
        return ActionResult::INVALID_POSITION;
    }
    
    // Intentar jugar la carta
    GameAction action;
    action.playerId = playerId;
    action.card = card;
    action.x = x;
    action.y = y;
    action.type = GameAction::ActionType::PLAY_CARD;
    
    bool success = gameState->processAction(action);
    
    return success ? ActionResult::SUCCESS : ActionResult::POSITION_OCCUPIED;
}

GameAPI::ActionResult GameAPI::moveCard(PlayerId playerId, uint8_t fromX, uint8_t fromY, uint8_t toX, uint8_t toY) {
    if (!isInitialized()) {
        return ActionResult::GAME_OVER;
    }
    
    if (gameState->isGameOver()) {
        return ActionResult::GAME_OVER;
    }
    
    if (playerId > 1) {
        return ActionResult::INVALID_PLAYER;
    }
    
    // Verificar si es el turno del jugador
    PlayerId currentPlayer = gameState->getCurrentPlayer();
    if (playerId != currentPlayer) {
        return ActionResult::INVALID_PLAYER;
    }
    
    if (gameState->getActionsRemaining(playerId) == 0) {
        return ActionResult::NOT_ENOUGH_ACTIONS;
    }
    
    // Verificar que hay una carta en la posición origen
    MapCell* fromCell = gameState->getMap().at(fromX, fromY);
    if (!fromCell || !fromCell->card.has_value()) {
        return ActionResult::INVALID_CARD;
    }
    
    CardPtr card = fromCell->card.value();
    if (card->getOwner() != playerId) {
        return ActionResult::INVALID_CARD;
    }
    
    // Intentar mover la carta
    GameAction action;
    action.playerId = playerId;
    action.card = card;
    action.x = toX;
    action.y = toY;
    action.type = GameAction::ActionType::MOVE_CARD;
    
    bool success = gameState->processAction(action);
    
    return success ? ActionResult::SUCCESS : ActionResult::INVALID_MOVE;
}

GameAPI::ActionResult GameAPI::attackCard(PlayerId playerId, uint8_t fromX, uint8_t fromY, uint8_t toX, uint8_t toY) {
    if (!isInitialized()) {
        return ActionResult::GAME_OVER;
    }
    
    if (gameState->isGameOver()) {
        return ActionResult::GAME_OVER;
    }
    
    if (playerId > 1) {
        return ActionResult::INVALID_PLAYER;
    }
    
    // Verificar si es el turno del jugador
    PlayerId currentPlayer = gameState->getCurrentPlayer();
    if (playerId != currentPlayer) {
        return ActionResult::INVALID_PLAYER;
    }
    
    if (gameState->getActionsRemaining(playerId) == 0) {
        return ActionResult::NOT_ENOUGH_ACTIONS;
    }
    
    // Verificar que hay una carta en la posición origen
    MapCell* fromCell = gameState->getMap().at(fromX, fromY);
    if (!fromCell || !fromCell->card.has_value()) {
        return ActionResult::INVALID_CARD;
    }
    
    CardPtr card = fromCell->card.value();
    if (card->getOwner() != playerId) {
        return ActionResult::INVALID_CARD;
    }
    
    // Intentar atacar
    GameAction action;
    action.playerId = playerId;
    action.card = card;
    action.type = GameAction::ActionType::ATTACK;
    action.target = std::make_pair(toX, toY);
    
    bool success = gameState->processAction(action);
    
    return success ? ActionResult::SUCCESS : ActionResult::CANNOT_ATTACK_TARGET;
}

GameAPI::ActionResult GameAPI::drawCard(PlayerId playerId) {
    if (!isInitialized()) {
        return ActionResult::GAME_OVER;
    }
    
    if (gameState->isGameOver()) {
        return ActionResult::GAME_OVER;
    }
    
    if (playerId > 1) {
        return ActionResult::INVALID_PLAYER;
    }
    
    // En este juego, las cartas se roban automáticamente al final del turno
    // Esta función podría usarse para robos adicionales especiales, pero
    // por ahora simplemente retornamos éxito si el jugador puede robar
    const Player& player = gameState->getPlayer(playerId);
    
    // Verificar que el jugador puede robar cartas (deck no vacío o discard no vacío)
    if (player.deck.empty() && player.discard.empty()) {
        return ActionResult::INVALID_CARD;
    }
    
    // Verificar que la mano no esté llena (asumiendo límite de 7 cartas)
    if (player.hand.size() >= 7) {
        return ActionResult::INVALID_CARD;
    }
    
    // Nota: El robo automático ocurre al final del turno, no consume acciones
    // Esta función es principalmente para validar si es posible robar
    return ActionResult::SUCCESS;
}

GameAPI::ActionResult GameAPI::castSpell(PlayerId playerId, size_t cardIndex) {
    if (!isInitialized()) {
        return ActionResult::GAME_OVER;
    }
    
    if (gameState->isGameOver()) {
        return ActionResult::GAME_OVER;
    }
    
    if (playerId > 1) {
        return ActionResult::INVALID_PLAYER;
    }
    
    // Verificar si es el turno del jugador
    PlayerId currentPlayer = gameState->getCurrentPlayer();
    if (playerId != currentPlayer) {
        return ActionResult::INVALID_PLAYER;
    }
    
    // Verificar si el jugador tiene acciones restantes
    if (gameState->getActionsRemaining(playerId) == 0) {
        return ActionResult::NOT_ENOUGH_ACTIONS;
    }
    
    // Obtener al jugador
    const Player& player = gameState->getPlayer(playerId);
    
    // Verificar que el índice de carta es válido
    if (cardIndex >= player.hand.size()) {
        return ActionResult::INVALID_CARD;
    }
    
    // Obtener la carta
    CardPtr card = player.hand[cardIndex];
    
    // Verificar que la carta es un hechizo
    auto spell = std::dynamic_pointer_cast<Spell>(card);
    if (!spell) {
        return ActionResult::INVALID_CARD; // Solo se pueden lanzar hechizos
    }
    
    // Crear acción de hechizo 
    GameAction action;
    action.playerId = playerId;
    action.card = card;
    action.x = 0; // Los hechizos no necesitan posición específica
    action.y = 0;
    action.type = GameAction::ActionType::PLAY_CARD;
    
    // Procesar la acción (esto maneja automáticamente los hechizos)
    bool success = gameState->processAction(action);
    
    return success ? ActionResult::SUCCESS : ActionResult::INVALID_CARD;
}

GameAPI::ActionResult GameAPI::endTurn() {
    if (!isInitialized()) {
        return ActionResult::GAME_OVER;
    }
    
    if (gameState->isGameOver()) {
        return ActionResult::GAME_OVER;
    }
    
    gameState->endTurn(gameState->getCurrentPlayer());
    return ActionResult::SUCCESS;
}

std::vector<std::pair<uint8_t, uint8_t>> GameAPI::getValidPlayPositions(PlayerId playerId) const {
    std::vector<std::pair<uint8_t, uint8_t>> positions;
    
    if (!isInitialized()) {
        return positions;
    }
    
    for (uint8_t y = 0; y < GameMap::MAP_HEIGHT; ++y) {
        for (uint8_t x = 0; x < GameMap::MAP_WIDTH; ++x) {
            if (isValidPlayPosition(playerId, x, y)) {
                positions.emplace_back(x, y);
            }
        }
    }
    
    return positions;
}

std::vector<std::pair<uint8_t, uint8_t>> GameAPI::getValidMovePositions(uint8_t x, uint8_t y) const {
    std::vector<std::pair<uint8_t, uint8_t>> positions;
    
    if (!isInitialized()) {
        return positions;
    }
    
    MapCell* fromCell = gameState->getMap().at(x, y);
    if (!fromCell || !fromCell->card.has_value()) {
        return positions;
    }
    
    CardPtr card = fromCell->card.value();
    auto unit = std::dynamic_pointer_cast<Unit>(card);
    if (!unit) {
        return positions;
    }
    
    // Para simplificar, asumimos que se puede mover a celdas adyacentes caminables y vacías
    for (uint8_t toY = 0; toY < GameMap::MAP_HEIGHT; ++toY) {
        for (uint8_t toX = 0; toX < GameMap::MAP_WIDTH; ++toX) {
            if (gameState->canMoveCard(card->getOwner(), card, x, y, toX, toY)) {
                positions.emplace_back(toX, toY);
            }
        }
    }
    
    return positions;
}

std::vector<std::pair<uint8_t, uint8_t>> GameAPI::getValidAttackPositions(uint8_t x, uint8_t y) const {
    std::vector<std::pair<uint8_t, uint8_t>> positions;
    
    if (!isInitialized()) {
        return positions;
    }
    
    MapCell* fromCell = gameState->getMap().at(x, y);
    if (!fromCell || !fromCell->card.has_value()) {
        return positions;
    }
    
    CardPtr card = fromCell->card.value();
    auto unit = std::dynamic_pointer_cast<Unit>(card);
    if (!unit) {
        return positions;
    }
    
    // Verificar todas las posiciones dentro del rango de ataque
    for (uint8_t toY = 0; toY < GameMap::MAP_HEIGHT; ++toY) {
        for (uint8_t toX = 0; toX < GameMap::MAP_WIDTH; ++toX) {
            if (gameState->canAttack(card->getOwner(), card, toX, toY)) {
                positions.emplace_back(toX, toY);
            }
        }
    }
    
    return positions;
}

bool GameAPI::isValidPlayPosition(PlayerId playerId, uint8_t x, uint8_t y) const {
    if (!isInitialized()) {
        return false;
    }
    
    // Verificar que la posición está dentro del mapa
    if (x >= GameMap::MAP_WIDTH || y >= GameMap::MAP_HEIGHT) {
        return false;
    }
    
    MapCell* cell = gameState->getMap().at(x, y);
    if (!cell) {
        return false;
    }
    
    // La celda debe ser caminable y estar vacía
    if ((cell->floor == MapCell::FloorType::NONE) || cell->card.has_value()) {
        return false;
    }
    
    // Verificar que está dentro del territorio del jugador (simplificado)
    // El jugador 0 puede jugar en las primeras 4 filas, el jugador 1 en las últimas 4
    if (playerId == 0 && y > 3) {
        return false;
    }
    if (playerId == 1 && y < 3) {
        return false;
    }
    
    // Verificar que la posición es adyacente al líder del equipo
    const Player& player = gameState->getPlayer(playerId);
    if (!player.legend || !player.legend->isOnMap()) {
        return false; // No se puede jugar sin líder en el mapa
    }
    
    auto legend = std::dynamic_pointer_cast<Unit>(player.legend);
    if (!legend) {
        return false;
    }
    
    auto [leaderX, leaderY] = legend->getCoordinates();
    
    // Verificar si la posición (x, y) es adyacente al líder
    // Usamos el sistema de adyacencia hexagonal del GameMap
    MapCell* leaderCell = gameState->getMap().at(leaderX, leaderY);
    if (!leaderCell) {
        return false;
    }
    
    // Verificar todas las direcciones de adyacencia
    for (int i = 0; i < 6; ++i) {
        GameMap::Adjacency adj = static_cast<GameMap::Adjacency>(i);
        MapCell* adjacentCell = gameState->getMap().getNeighbor(adj, leaderCell, playerId);
        if (adjacentCell && adjacentCell == cell) {
            return true; // La posición es adyacente al líder
        }
    }
    
    return false; // La posición no es adyacente al líder
}

std::string GameAPI::actionResultToString(ActionResult result) {
    switch (result) {
        case ActionResult::SUCCESS: return "SUCCESS";
        case ActionResult::INVALID_PLAYER: return "INVALID_PLAYER";
        case ActionResult::INVALID_POSITION: return "INVALID_POSITION";
        case ActionResult::INVALID_CARD: return "INVALID_CARD";
        case ActionResult::NOT_ENOUGH_ACTIONS: return "NOT_ENOUGH_ACTIONS";
        case ActionResult::POSITION_OCCUPIED: return "POSITION_OCCUPIED";
        case ActionResult::CARD_NOT_IN_HAND: return "CARD_NOT_IN_HAND";
        case ActionResult::CANNOT_ATTACK_TARGET: return "CANNOT_ATTACK_TARGET";
        case ActionResult::OUT_OF_RANGE: return "OUT_OF_RANGE";
        case ActionResult::GAME_OVER: return "GAME_OVER";
        case ActionResult::INVALID_MOVE: return "INVALID_MOVE";
        default: return "UNKNOWN";
    }
}

GameAPI::PlayerInfo GameAPI::extractPlayerInfo(const Player& player) const {
    PlayerInfo info;
    info.id = player.id;
    info.team = player.team;
    info.name = player.name;
    info.isAlive = player.isAlive();
    info.actionsLeft = player.actionsRemaining;
    info.maxActions = player.maxActionsPerTurn;
    info.handSize = player.hand.size();
    info.deckSize = player.deck.size();
    
    // Obtener nombres de cartas en la mano
    for (const auto& card : player.hand) {
        if (card) {
            info.handCardNames.push_back(card->getName());
        } else {
            info.handCardNames.push_back("(null)");
        }
    }
    
    // Verificar si tiene leyenda y su posición
    info.hasLegend = (player.legend != nullptr);
    if (info.hasLegend) {
        info.legendName = player.legend->getName();
        auto legend = std::dynamic_pointer_cast<Unit>(player.legend);
        if (legend && legend->isOnMap()) {
            auto coords = legend->getCoordinates();
            info.legendPosition = {coords.first, coords.second};
        } else {
            info.legendPosition = {255, 255}; // Posición inválida
        }
    }
    
    return info;
}

GameAPI::CellInfo GameAPI::extractCellInfo(uint8_t x, uint8_t y) const {
    CellInfo info;
    info.x = x;
    info.y = y;
    
    MapCell* cell = gameState->getMap().at(x, y);
    if (!cell) {
        info.isWalkable = false;
        info.isSpawn = false;
        info.hasCard = false;
        return info;
    }
    
    info.isWalkable = (cell->floor != MapCell::FloorType::NONE);
    info.isSpawn = cell->isSpawn();
    info.hasCard = cell->card.has_value();
    
    if (info.hasCard) {
        CardPtr card = cell->card.value();
        info.cardName = card->getName();
        
        // Determinar el tipo de carta usando dynamic_pointer_cast
        if (std::dynamic_pointer_cast<Legend>(card)) {
            info.cardType = "Legend";
        } else if (std::dynamic_pointer_cast<Unit>(card)) {
            info.cardType = "Unit";
        } else if (std::dynamic_pointer_cast<Spell>(card)) {
            info.cardType = "Spell";
        } else {
            info.cardType = "Unknown";
        }
        
        info.cardOwner = card->getOwner();
        
        if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
            info.attack = unit->getAttack();
            info.health = unit->getHealth();
            info.speed = unit->getSpeed();
            info.range = unit->getRange();
            info.isLegend = unit->isLegend();
        } else {
            info.attack = 0;
            info.health = 0;
            info.speed = 0;
            info.range = 0;
            info.isLegend = false;
        }
    } else {
        info.cardOwner = 255; // Valor inválido
        info.attack = 0;
        info.health = 0;
        info.speed = 0;
        info.range = 0;
        info.isLegend = false;
    }
    
    return info;
}
