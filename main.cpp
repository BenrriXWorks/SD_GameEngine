#include <print>
#include <array>
#include <functional>
#include <utility>
#include <string>
#include "src/Effect.hpp"
#include "src/MapCell.hpp"
#include "src/GameMap.hpp"
#include "src/EffectStack.hpp"
#include "src/EffectImpl/PrintEffect.hpp"

using namespace std;


int main() {
    EffectStack stack;
    GameMap gameMap = GameMap();
    MapCell* sourceCell = gameMap.at(2, 0);
    list<MapCell*> targets = { sourceCell };    
    uint8_t card1 = 42;
    uint8_t card2 = 99;    

    // Agrega efectos de ejemplo
    stack.addEffect(TriggerAction::ON_PLAY, 1001, make_shared<PrintEffect>(card1, "Summon Buff"));
    stack.addEffect(TriggerAction::ON_PLAY, 1002, make_shared<PrintEffect>(card2, "Draw Shield"));

    println("Initial Effect Stack:\n{}\n", string(stack));

    println("Triggering ON_PLAY effects:");
    stack.triggerEffects(TriggerAction::ON_PLAY, gameMap, sourceCell, targets);

    println("\nAfter triggering:\n{}\n", string(stack));

    // Reagrega uno para probar remove
    stack.addEffect(TriggerAction::ON_ATTACK, 1001, make_shared<PrintEffect>(card1, "Attack Boost"));

    println("Before removing effects from sourceID 1001:\n{}\n", string(stack));
    stack.removeEffectsBySource(1001);
    println("After removing effects from sourceID 1001:\n{}", string(stack));

    return 0;
}