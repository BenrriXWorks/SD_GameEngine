#pragma once
#include <list>
#include <functional>
#include <array>
#include <cstdint>
#include <string>
#include <memory>
#include "../libs/flat_hash_map.hpp"
#include "Effect.hpp"
#include "GameMap.hpp"

enum class TriggerAction : uint8_t {
    ON_PLAY = 0,
    ON_MOVE = 1,
    ON_ATTACK = 2,
    ON_COMBAT = 3,
    ON_DRAW = 4,
    ON_END_OF_TURN = 5,
    ON_START_OF_TURN= 6,
    CAST_EFFECT = 7,
    ON_DESTRUCTION = 8
};

constexpr std::array TRIGGER_ACTION_NAMES = {
    "ON_PLAY", "ON_ATTACK", "ON_COMBAT", "ON_DRAW", "ON_END_OF_TURN", "ON_START_OF_TURN", "CAST_EFFECT", "ON_DESTRUCTION"
};

inline std::string getTriggerActionName(TriggerAction action) {
    return TRIGGER_ACTION_NAMES[static_cast<uint8_t>(action)];
}

constexpr const uint8_t TRIGGER_COUNT = TRIGGER_ACTION_NAMES.size();



class EffectStack {
    using EffectPtr = std::shared_ptr<Effect>;
    std::array<std::list<std::pair<uint64_t, EffectPtr>>, TRIGGER_COUNT> listeners;

    public:        void addEffect(TriggerAction action, uint64_t sourceID, EffectPtr effect) {
            listeners[static_cast<uint8_t>(action)].emplace_back(sourceID, effect);
        }

        void triggerEffects(TriggerAction action, GameMap& gameMap, MapCell* sourceCell, std::list<MapCell*> targetCells) {
            auto& effectList = listeners[(uint8_t)action];
            for (auto it = effectList.begin(); it != effectList.end(); ) {
                bool keep = it->second->check(gameMap, sourceCell, targetCells);
                it = keep ? std::next(it) : effectList.erase(it);
            }
        }

        void removeEffectsBySource(uint64_t sourceID) {
            for (auto& effectList : listeners) {
                effectList.remove_if([&](auto& pair) {
                    return pair.first == sourceID;
                });
            }
        }

        operator std::string() const {
            std::string result;
            for (size_t i = 0; i < listeners.size(); ++i) {
                result += getTriggerActionName((TriggerAction)i) + ":\n";
                for (const auto& [src, effect] : listeners[i]) {
                    result += "  From " + std::to_string(src) + ": " + effect->name + "\n";
                }
            }
            return result;
        }
};
