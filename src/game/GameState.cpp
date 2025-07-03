#include "GameState.hpp"
#include <print>
#include <stdexcept>
#include <chrono>
#include <thread>

// Thread-local RNG state for maximum efficiency
// Initialize with a unique value to avoid same seeds across threads
thread_local uint32_t GameState::rng_state = []() {
    // Combine multiple entropy sources for better uniqueness
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    auto thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
    
    // Mix stack address for additional entropy (each thread has different stack)
    auto stack_addr = reinterpret_cast<uintptr_t>(&now);
    
    // Combine all entropy sources with different bit shifts
    return static_cast<uint32_t>(nanos) ^ 
           static_cast<uint32_t>(thread_id << 16) ^ 
           static_cast<uint32_t>(stack_addr >> 8) ^
           static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count());
}();

GameState::GameState() : 
    map(), // Create default map
    effectStack(),
    gameConfig() {  // Use default config
    // Initialize with default values
    phase = GamePhase::SETUP;
    turnNumber = 0;
    currentPlayer = 0;
    
    // RNG is already initialized with unique seed per thread
    // Warm it up with a few iterations for better distribution
    for (int i = 0; i < 5; ++i) {
        fastRand();
    }
}

GameState::GameState(const GameConfig& config) : 
    map(), // Create default map
    effectStack(),
    gameConfig(config) {
    // Initialize with default values
    phase = GamePhase::SETUP;
    turnNumber = 0;
    currentPlayer = 0;
    
    // RNG is already initialized with unique seed per thread
    // Warm it up with a few iterations for better distribution
    for (int i = 0; i < 5; ++i) {
        fastRand();
    }
}

GameState::GameState(std::vector<CardPtr> deck1, std::vector<CardPtr> deck2, const GameConfig& config) :
    map(),
    effectStack(),
    gameConfig(config) {
    // Initialize with default values
    phase = GamePhase::SETUP;
    turnNumber = 0;
    currentPlayer = 0;
    
    // Add two players with their respective decks
    addPlayer(0, Team::TEAM_A, "Player 1");
    addPlayer(1, Team::TEAM_B, "Player 2");
    
    setPlayerDeck(0, std::move(deck1));
    setPlayerDeck(1, std::move(deck2));
    
    // Apply configuration to players
    applyConfigurationToPlayers();
    
    // Start the game automatically
    startGame();
    
    // RNG warmup
    for (int i = 0; i < 5; ++i) {
        fastRand();
    }
}

void GameState::addPlayer(PlayerId id, Team team, const std::string& name) {
    players.emplace_back(id, team, name);
}

void GameState::setPlayerDeck(PlayerId id, std::vector<CardPtr> deck) {
    // Use simple player lookup
    Player* player = findPlayer(id);
    if (player) {
        player->deck = std::move(deck);
        // Shuffle the deck using fast RNG
        shuffleContainer(player->deck);
    } else {
        std::println("Player {} not found!", id);
    }
}

void GameState::startGame() {
    // Aplicar configuración a los jugadores antes de empezar
    applyConfigurationToPlayers();
    
    // PRIMERO: Colocar las leyendas en sus posiciones de spawn
    // y removerlas de los mazos ANTES de robar cualquier carta
    placeLegends();
    
    // SEGUNDO: Robar cartas iniciales usando configuración
    uint8_t initialHandSize = gameConfig.getUInt8("initial_hand_size", 5);
    for (auto& player : players) {
        drawCard(player, initialHandSize);
        player.actionsRemaining = player.maxActionsPerTurn;
    }
    
    phase = GamePhase::PLAY;
    currentPlayer = 0; // First player starts
}

void GameState::drawCard(Player& player, uint8_t count) {
    for (uint8_t i = 0; i < count; ++i) {
        // Respetar el límite de mano
        if (player.hand.size() >= player.maxHandSize) {
            break;
        }
        
        if (player.deck.empty()) {
            // Reshuffle discard pile into deck if needed
            if (!player.discard.empty()) {
                player.deck = std::move(player.discard);
                shuffleContainer(player.deck);
            } else {
                // No cards left to draw
                break;
            }
        }
        
        if (!player.deck.empty()) {
            player.hand.push_back(player.deck.back());
            player.deck.pop_back();
        }
    }
}

void GameState::returnCardToDeck(PlayerId playerId, CardPtr card) {
    Player* player = findPlayer(playerId);
    if (!player || !card) {
        return;
    }
    
    // Las leyendas NUNCA vuelven al mazo - son únicas
    if (auto legend = std::dynamic_pointer_cast<Legend>(card)) {
        std::println("⚠️  Advertencia: Intentando devolver leyenda {} al mazo - las leyendas no se reciclan", legend->getName());
        return;
    }
    
    // Agregar la carta al mazo
    player->deck.push_back(card);
    
    // Barajar el mazo para que la carta aparezca en una posición aleatoria
    shuffleContainer(player->deck);
    
    std::println("Card {} returned to {}'s deck and shuffled", card->getName(), player->name);
}

void GameState::endTurn(PlayerId playerId) {
    if (playerId != currentPlayer) {
        std::println("Not your turn!");
        return;
    }
    
    // Reset current player's actions for next turn
    resetPlayerActions(currentPlayer);
    
    // Move to next player
    currentPlayer = (currentPlayer + 1) % players.size();
    
    // If we've gone through all players, increment turn number
    if (currentPlayer == 0) {
        turnNumber++;
    }
    
    // Process end-of-turn effects
    effectStack.processEndOfTurn(map);
    
    // Give the new current player a card and reset actions
    Player* player = findPlayer(currentPlayer);
    if (player) {
        drawCard(*player, 1);
        player->actionsRemaining = player->maxActionsPerTurn;
    }
}

void GameState::applyConfigurationToPlayers() {
    uint8_t maxHandSize = gameConfig.getUInt8("max_hand_size", 7);
    uint8_t maxActions = gameConfig.getUInt8("max_actions_per_turn", 3);
    uint8_t initialHealth = gameConfig.getUInt8("initial_health", 20);
    
    for (auto& player : players) {
        player.maxHandSize = maxHandSize;
        player.maxActionsPerTurn = maxActions;
        player.health = initialHealth;
        player.actionsRemaining = maxActions; // Set current actions to max
    }
}

bool GameState::processAction(const GameAction& action) {
    if (action.playerId != currentPlayer) {
        std::println("Not your turn!");
        return false;
    }
    
    // Check if player has actions remaining (except for END_TURN)
    if (action.type != GameAction::ActionType::END_TURN && !hasActionsRemaining(action.playerId)) {
        std::println("No actions remaining! Use END_TURN to end your turn.");
        return false;
    }
    
    bool success = false;
    switch (action.type) {
        case GameAction::ActionType::PLAY_CARD:
            playCard(action.playerId, action.card, action.x, action.y);
            success = true;
            break;
        case GameAction::ActionType::MOVE_CARD:
            success = moveCard(action.playerId, action.card, action.x, action.y);
            break;
        case GameAction::ActionType::ATTACK:
            if (action.target.has_value()) {
                auto [targetX, targetY] = action.target.value();
                success = attackWithCard(action.playerId, action.card, targetX, targetY);
            }
            break;
        case GameAction::ActionType::END_TURN:
            endTurn(action.playerId);
            return true; // END_TURN doesn't consume actions
        default:
            return false;
    }
    
    // Consume action only if the action was successful
    if (success) {
        consumeAction(action.playerId);
        // Verificar estado de leyendas después de cada acción
        checkLegendStatus();
    }
    
    return success;
}

void GameState::playCard(PlayerId playerId, CardPtr card, uint8_t x, uint8_t y) {
    // Use simple player lookup
    Player* player = findPlayer(playerId);
    if (!player) {
        std::println("Player {} not found!", playerId);
        return;
    }

    // Check if player has the card in hand (optimized for small hand size)
    size_t cardIndex = player->findCardIndex(card);
    if (cardIndex == SIZE_MAX) {
        std::println("Card not in player's hand!");
        return;
    }

    // Verificar que no sea una leyenda - las leyendas no se pueden jugar desde la mano
    if (auto legend = std::dynamic_pointer_cast<Legend>(card)) {
        std::println("❌ Error: Las leyendas no se pueden jugar desde la mano. La leyenda {} ya debería estar en el mapa.", legend->getName());
        return;
    }

    // Remove from hand first (efficient removal by index)
    player->hand.erase(player->hand.begin() + cardIndex);

    // Handle different card types
    if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
        // For units: place on the map
        MapCell* cell = map.at(x, y);
        if (!cell || cell->card.has_value() || cell->floor == MapCell::FloorType::NONE) {
            std::println("Invalid target position for unit! Position ({}, {}) is not walkable or occupied", x, y);
            // Return card to hand if placement failed
            player->hand.insert(player->hand.begin() + cardIndex, card);
            return;
        }
        
        // Para unidades, verificar adyacencia al líder
        if (!player->legend || !player->legend->isOnMap()) {
            std::println("❌ Error: No se puede colocar unidades sin un líder en el mapa.");
            player->hand.insert(player->hand.begin() + cardIndex, card);
            return;
        }
        
        auto legend = std::dynamic_pointer_cast<Unit>(player->legend);
        if (!legend) {
            std::println("❌ Error: Problema con el líder del jugador.");
            player->hand.insert(player->hand.begin() + cardIndex, card);
            return;
        }
        
        auto [leaderX, leaderY] = legend->getCoordinates();
        MapCell* leaderCell = map.at(leaderX, leaderY);
        
        bool isAdjacent = false;
        // Verificar todas las direcciones de adyacencia
        for (int i = 0; i < 6; ++i) {
            GameMap::Adjacency adj = static_cast<GameMap::Adjacency>(i);
            MapCell* adjacentCell = map.getNeighbor(adj, leaderCell, playerId);
            if (adjacentCell && adjacentCell == cell) {
                isAdjacent = true;
                break;
            }
        }
        
        if (!isAdjacent) {
            std::println("❌ Error: Las unidades solo pueden colocarse adyacentes al líder del jugador.");
            player->hand.insert(player->hand.begin() + cardIndex, card);
            return;
        }

        // Set owner and place the unit on the map
        card->setOwner(playerId);
        cell->card = card;
        std::println("Unit {} played at position ({}, {})", card->getName(), x, y);

    } else if (auto spell = std::dynamic_pointer_cast<Spell>(card)) {
        // For spells: cast immediately and return to deck
        std::println("Spell {} cast", card->getName());

        // Process the spell's effects immediately
        for (const auto& effect : card->getEffects()) {
            effectStack.addEffect(effect);
        }

        // Return spell to deck after casting
        returnCardToDeck(playerId, card);

    } else {
        std::println("Unknown card type for {}", card->getName());
        // Return card to hand if unknown type
        player->hand.insert(player->hand.begin() + cardIndex, card);
        return;
    }
    
    // Process remaining effects (for units)
    if (auto unit = std::dynamic_pointer_cast<Unit>(card)) {
        for (const auto& effect : card->getEffects()) {
            effectStack.addEffect(effect);
        }
    }
    
    // Consume action after successful play
    consumeAction(playerId);
}

// Placeholder implementations for remaining methods
bool GameState::moveCard(PlayerId playerId, CardPtr card, uint8_t x, uint8_t y) {
    // Find current position of the card
    uint8_t fromX = 255, fromY = 255; // Invalid positions as default
    
    for (uint8_t mapY = 0; mapY < map.getHeight(); ++mapY) {
        for (uint8_t mapX = 0; mapX < map.getWidth(); ++mapX) {
            MapCell* cell = map.at(mapX, mapY);
            if (cell && cell->card.has_value() && cell->card.value() == card) {
                fromX = mapX;
                fromY = mapY;
                break;
            }
        }
        if (fromX != 255) break;
    }
    
    if (fromX == 255) {
        std::println("Card {} not found on map!", card->getName());
        return false;
    }
    
    // Validate movement
    if (!canMoveCard(playerId, card, fromX, fromY, x, y)) {
        std::println("Cannot move card {} from ({}, {}) to ({}, {})", 
                     card->getName(), fromX, fromY, x, y);
        return false;
    }
    
    // Perform the move
    MapCell* fromCell = map.at(fromX, fromY);
    MapCell* toCell = map.at(x, y);
    
    toCell->card = card;
    fromCell->card.reset();
    // Consume action after successful move
    consumeAction(playerId);
    std::println("Moved card {} from ({}, {}) to ({}, {})", 
                 card->getName(), fromX, fromY, x, y);
    return true;
}

bool GameState::attackWithCard(PlayerId playerId, CardPtr card, uint8_t targetX, uint8_t targetY) {
    // Validate attack using improved validation
    if (!canAttack(playerId, card, targetX, targetY)) {
        std::println("Invalid attack by player {} with card {}", playerId, card ? card->getName() : "null");
        return false;
    }
    
    // Get target
    MapCell* targetCell = map.at(targetX, targetY);
    CardPtr target = targetCell->card.value();
    
    // Simple combat: destroy target (placeholder - you'd want actual stats)
    std::println("Player {} attacks with {} targeting {} at position ({}, {})", 
                 playerId, card->getName(), target->getName(), targetX, targetY);
    
    // Destroy the target card
    destroyCard(target);
    // Consume action after successful attack
    consumeAction(playerId);
    return true;
}

// Game state validation and win conditions
bool GameState::isGameOver() const {
    // El juego termina si algún jugador no tiene leyenda viva
    for (const auto& player : players) {
        if (!player.isAlive()) {
            return true;
        }
    }
    return false;
}

std::optional<Team> GameState::getWinner() const {
    if (!isGameOver()) {
        return std::nullopt;
    }
    
    // Find the team with living legends
    for (const auto& player : players) {
        if (player.isAlive()) {
            return player.team;
        }
    }
    
    return std::nullopt; // Draw (ambos sin leyenda)
}

bool GameState::isPlayerAlive(PlayerId playerId) const {
    const Player* player = findPlayer(playerId);
    return player && player->isAlive();
}

// Combat system
bool GameState::canAttack(PlayerId playerId, CardPtr attacker, uint8_t targetX, uint8_t targetY) const {
    // Basic validation - card must exist and be owned by the player
    if (!attacker || attacker->getOwner() != playerId) {
        return false;
    }
    
    // Check if attacking card is on the map
    bool attackerOnMap = false;
    for (uint8_t y = 0; y < map.getHeight() && !attackerOnMap; ++y) {
        for (uint8_t x = 0; x < map.getWidth() && !attackerOnMap; ++x) {
            const MapCell* cell = map.at(x, y);
            if (cell && cell->card.has_value() && cell->card.value() == attacker) {
                attackerOnMap = true;
                break;
            }
        }
    }
    
    if (!attackerOnMap) {
        return false;
    }
    
    // Check if target position is valid
    if (!isValidPosition(targetX, targetY)) {
        return false;
    }
    
    // Check if there's something to attack at target position
    const MapCell* targetCell = map.at(targetX, targetY);
    if (!targetCell || !targetCell->card.has_value()) {
        return false;
    }
    
    // Can't attack own cards
    CardPtr target = targetCell->card.value();
    if (target->getOwner() == playerId) {
        return false;
    }
    
    return true;
}

void GameState::dealDamage(PlayerId targetPlayer, uint8_t damage) {
    Player* player = findPlayer(targetPlayer);
    if (player) {
        if (damage >= player->health) {
            player->health = 0;
        } else {
            player->health -= damage;
        }
        std::println("Player {} takes {} damage, health now: {}", targetPlayer, damage, player->health);
    }
}

void GameState::destroyCard(CardPtr card) {
    if (!card) return;
    
    // Find the card on the map and remove it
    for (uint8_t y = 0; y < map.getHeight(); ++y) {
        for (uint8_t x = 0; x < map.getWidth(); ++x) {
            MapCell* cell = map.at(x, y);
            if (cell && cell->card.has_value() && cell->card.value() == card) {
                // Return card to owner's deck (except legends)
                Player* owner = findPlayer(card->getOwner());
                if (owner) {
                    // Si es una leyenda, manejar especialmente
                    if (auto legend = std::dynamic_pointer_cast<Legend>(card)) {
                        if (owner->legend == legend) {
                            owner->legend = nullptr;
                            std::println("¡Leyenda {} destruida! Jugador {} eliminado!", 
                                       legend->getName(), owner->id);
                            
                            // Las leyendas destruidas NO regresan al mazo (son únicas)
                            owner->discard.push_back(card);
                        }
                    } else {
                        // Cartas normales regresan al mazo para ser reutilizadas
                        returnCardToDeck(owner->id, card);
                    }
                }
                cell->card.reset();
                std::println("Card {} destroyed", card->getName());
                
                // Verificar estado después de destruir una carta
                checkLegendStatus();
                return;
            }
        }
    }
}

// Movement and positioning validation
bool GameState::canMoveCard(PlayerId playerId, CardPtr card, uint8_t fromX, uint8_t fromY, uint8_t toX, uint8_t toY) const {
    // Basic movement validation
    if (!isValidPosition(fromX, fromY) || !isValidPosition(toX, toY)) {
        return false;
    }
    
    // Check if card is at source position and owned by player
    const MapCell* fromCell = map.at(fromX, fromY);
    if (!fromCell || !fromCell->card.has_value() || fromCell->card.value() != card) {
        return false;
    }
    
    if (card->getOwner() != playerId) {
        return false;
    }
    
    // Check if destination is empty
    return isPositionEmpty(toX, toY);
}

bool GameState::isValidPosition(uint8_t x, uint8_t y) const {
    const MapCell* cell = map.at(x, y);
    return cell != nullptr && cell->floor != MapCell::FloorType::NONE;
}

bool GameState::isPositionEmpty(uint8_t x, uint8_t y) const {
    const MapCell* cell = map.at(x, y);
    return cell && cell->floor != MapCell::FloorType::NONE && !cell->card.has_value();
}

// Simple helper methods for small number of players
Player* GameState::findPlayer(PlayerId id) {
    for (auto& player : players) {
        if (player.id == id) {
            return &player;
        }
    }
    return nullptr;
}

const Player* GameState::findPlayer(PlayerId id) const {
    for (const auto& player : players) {
        if (player.id == id) {
            return &player;
        }
    }
    return nullptr;
}

const Player& GameState::getPlayer(PlayerId id) const {
    const Player* player = findPlayer(id);
    if (!player) {
        throw std::runtime_error("Player not found: " + std::to_string(id));
    }
    return *player;
}

bool GameState::hasActionsRemaining(PlayerId playerId) const {
    const Player* player = findPlayer(playerId);
    return player && player->actionsRemaining > 0;
}

bool GameState::consumeAction(PlayerId playerId) {
    Player* player = findPlayer(playerId);
    if (player && player->actionsRemaining > 0) {
        player->actionsRemaining--;
        return true;
    }
    return false;
}

void GameState::resetPlayerActions(PlayerId playerId) {
    Player* player = findPlayer(playerId);
    if (player) {
        player->actionsRemaining = player->maxActionsPerTurn;
    }
}

uint8_t GameState::getActionsRemaining(PlayerId playerId) const {
    const Player* player = findPlayer(playerId);
    return player ? player->actionsRemaining : 0;
}

uint8_t GameState::getMaxActionsPerTurn(PlayerId playerId) const {
    const Player* player = findPlayer(playerId);
    return player ? player->maxActionsPerTurn : 0;
}

void GameState::setMaxActionsPerTurn(PlayerId playerId, uint8_t maxActions) {
    Player* player = findPlayer(playerId);
    if (player) {
        player->maxActionsPerTurn = maxActions;
        // If current actions exceed new max, cap them
        if (player->actionsRemaining > maxActions) {
            player->actionsRemaining = maxActions;
        }
    }
}

// Player hand optimization methods
size_t Player::findCardIndex(CardPtr card) const {
    for (size_t i = 0; i < hand.size(); ++i) {
        if (hand[i]->getId() == card->getId()) {
            return i;
        }
    }
    return SIZE_MAX; // Not found
}

bool Player::hasCard(CardPtr card) const {
    return findCardIndex(card) != SIZE_MAX;
}

// Fast thread-local random number generation
uint32_t GameState::fastRand() {
    // Very fast LCG with good statistical properties
    // Using constants from Numerical Recipes
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

void GameState::seedRng(uint32_t seed) {
    if (seed == 0) {
        // Generate a new unique seed similar to the thread_local initialization
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
        auto thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
        auto stack_addr = reinterpret_cast<uintptr_t>(&seed);
        
        // Add some variation by including a counter to make repeated calls different
        static thread_local uint32_t call_counter = 0;
        call_counter++;
        
        rng_state = static_cast<uint32_t>(nanos) ^ 
                   static_cast<uint32_t>(thread_id << 16) ^ 
                   static_cast<uint32_t>(stack_addr >> 8) ^
                   static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()) ^
                   (call_counter << 24);
    } else {
        rng_state = seed;
    }
    
    // Warm up the generator (some LCGs need a few iterations)
    for (int i = 0; i < 10; ++i) {
        fastRand();
    }
}

template<typename Container>
void GameState::shuffleContainer(Container& container) {
    // Fisher-Yates shuffle using our fast RNG
    for (size_t i = container.size() - 1; i > 0; --i) {
        size_t j = fastRand() % (i + 1);
        std::swap(container[i], container[j]);
    }
}

// Explicit instantiation for common container types
template void GameState::shuffleContainer<std::vector<CardPtr>>(std::vector<CardPtr>&);

// Legend management methods
void GameState::placeLegends() {
    std::println("=== INICIANDO COLOCACIÓN DE LEYENDAS ===");
    
    for (auto& player : players) {
        // Verificar si el jugador ya tiene una leyenda colocada
        if (player.legend) {
            std::println("⚠️  Jugador {} ya tiene una leyenda: {}", player.id, player.legend->getName());
            continue;
        }
        
        std::println("Procesando jugador {}, deck size: {}", player.id, player.deck.size());
        
        // Buscar leyenda en el deck del jugador
        auto legend = findLegendInDeck(player.deck);
        if (!legend) {
            std::println("⚠️  ¡Advertencia! Jugador {} no tiene leyenda en su deck", player.id);
            
            // Como fallback, crear una leyenda básica
            legend = std::make_shared<Legend>(
                999 + player.id, // ID único
                "Leyenda Básica " + std::to_string(player.id),
                0, // cost
                "Leyenda creada automáticamente",
                player.id, // owner
                3, // attack
                5, // health  
                1, // speed
                1  // range
            );
            std::println("🔧 Creada leyenda de emergencia para jugador {}: {}", player.id, legend->getName());
        } else {
            std::println("✅ Leyenda {} encontrada y removida del deck del jugador {}", legend->getName(), player.id);
        }
        
        // Obtener posición de spawn para este jugador
        auto [spawnX, spawnY] = map.getSpawnPosition(player.id);
        if (spawnX == 255) {
            std::println("❌ ¡Error! No hay posición de spawn para jugador {}", player.id);
            continue;
        }
        
        std::println("Posición de spawn para jugador {}: ({}, {})", player.id, spawnX, spawnY);
        
        // Verificar que la posición de spawn esté libre
        MapCell* spawnCell = map.at(spawnX, spawnY);
        if (!spawnCell) {
            std::println("❌ ¡Error! Celda de spawn ({}, {}) no existe para jugador {}", 
                       spawnX, spawnY, player.id);
            continue;
        }
        
        if (spawnCell->card.has_value()) {
            std::println("❌ ¡Error! Posición de spawn ({}, {}) ocupada para jugador {}", 
                       spawnX, spawnY, player.id);
            continue;
        }
        
        // Colocar la leyenda en la posición de spawn
        spawnCell->card = legend;
        legend->setPosition(spawnX, spawnY);
        legend->setOwner(player.id);
        
        // Guardar referencia a la leyenda del jugador
        player.legend = legend;
        
        std::println("✅ Leyenda {} colocada en spawn ({}, {}) para jugador {}", 
                   legend->getName(), spawnX, spawnY, player.id);
    }
    
    std::println("=== COLOCACIÓN DE LEYENDAS COMPLETADA ===");
}

std::shared_ptr<Legend> GameState::findLegendInDeck(std::vector<CardPtr>& deck) {
    std::shared_ptr<Legend> foundLegend = nullptr;
    
    // Buscar y remover TODAS las leyendas del deck (pueden haber duplicadas)
    auto it = deck.begin();
    while (it != deck.end()) {
        if (auto legend = std::dynamic_pointer_cast<Legend>(*it)) {
            if (!foundLegend) {
                // Guardar la primera leyenda encontrada para devolverla
                foundLegend = legend;
            }
            // Remover esta leyenda del deck
            it = deck.erase(it);
        } else {
            ++it;
        }
    }
    
    return foundLegend;
}

void GameState::checkLegendStatus() {
    if (isGameOver()) {
        auto winner = getWinner();
        if (winner.has_value()) {
            std::println("¡Juego terminado! Ganador: Team {}", 
                       static_cast<int>(winner.value()));
        } else {
            std::println("¡Juego terminado en empate!");
        }
        phase = GamePhase::END;
    }
}

bool Player::drawCard() {
    // No se puede robar si la mano está llena
    if (hand.size() >= maxHandSize) {
        return false;
    }
    
    // No se puede robar si el mazo está vacío
    if (deck.empty()) {
        // Intentar reabastecer desde el descarte
        if (!discard.empty()) {
            deck = std::move(discard);
            // Aquí deberíamos barajar, pero por simplicidad lo omitimos
            // En una implementación completa, usaríamos GameState::shuffleContainer
        } else {
            return false; // No hay cartas disponibles
        }
    }
    
    if (!deck.empty()) {
        hand.push_back(deck.back());
        deck.pop_back();
        return true;
    }
    
    return false;
}
