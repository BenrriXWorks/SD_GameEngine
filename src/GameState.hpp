#pragma once
#include <array>
#include <cstdint>
#include <utility>
#include <vector>
#include <thread>
#include <list>
#include "GameMap.hpp"
#include "EffectStack.hpp"

using PlayerId = uint32_t;
using Card = uint8_t;

enum class Team : uint8_t {
    NONE = 0,
    TEAM_A = 1,
    TEAM_B = 2
};

struct Player {
    Team team = Team::NONE;
    uint8_t maxActions = 3;
    uint8_t actionsLeft = 3;
    std::vector<Card> deck = {};
    Card leader = 1;
};

class GameState {
    public:
        GameMap gameMap;
        std::vector<Player> players;
        Team currentTeamInTurn;


        uint32_t turn = 0;
        uint32_t playedEffects = 0;

        EffectStack effectStack;

        enum class GamePhase : uint8_t {
            START_OF_TURN = 0,
            MID_TURN = 1,
            END_OF_TURN = 2
        };

        uint32_t stateRand() {
            thread_local uint32_t s = 0x9E3779B9 ^ std::hash<std::thread::id>{}(std::this_thread::get_id());
            s ^= s << 13; s ^= s >> 17; s ^= s << 5;
            return s;
        };



        GameState() {
            gameMap = GameMap();
            effectStack = EffectStack();
            players = std::vector<Player>(2); // Default to 2 players
            currentTeamInTurn = stateRand() & 1 ? Team::TEAM_A : Team::TEAM_B;

            GameMap().at(2, 0)->card = players[0].leader;
            effectStack.addEffect(TriggerAction::ON_PLAY, 0, std::make_unique<Effect>());
            GameMap().at(2, 6)->card = players[1].leader;

            effectStack.triggerEffects(TriggerAction::ON_START_OF_TURN, gameMap, gameMap.at(0, 0), std::list<MapCell*>{});
        }
        
};