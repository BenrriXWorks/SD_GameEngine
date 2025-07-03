#pragma once
#include "Effect.hpp"
#include "EffectImpl.hpp"
#include "../triggers/Trigger.hpp"
#include "../targeting/TargetSelector.hpp"
#include <vector>

// Forward declaration
class GameState;

// Clase para efectos compuestos (que usan el nuevo sistema de triggers y targeting)
class ComposedEffect : public Effect {
private:
    EffectImplPtr effectImpl;
    std::vector<TriggerPtr> triggers;
    TargetSelectorPtr targetSelector;

public:
    // Constructor con un solo trigger
    ComposedEffect(EffectImplPtr impl, TriggerPtr trigger, TargetSelectorPtr selector, CardPtr src, PlayerId ownerId) 
        : Effect(src, ownerId, impl->getName()), effectImpl(impl), targetSelector(selector) {
        triggers.push_back(trigger);
    }
    
    // Constructor con múltiples triggers
    ComposedEffect(EffectImplPtr impl, const std::vector<TriggerPtr>& triggerList, TargetSelectorPtr selector, CardPtr src, PlayerId ownerId) 
        : Effect(src, ownerId, impl->getName()), effectImpl(impl), triggers(triggerList), targetSelector(selector) {}
    
    // Implementar la verificación de efectos compuestos
    bool checkEffect(GameMap& gameMap, MapCell* triggerCell, std::list<MapCell*> targetCells) override {
        (void)targetCells; // Suprimir warning de parámetro no usado
        
        // Verificar si alguno de los triggers debe activarse
        bool shouldActivate = false;
        for (auto& trigger : triggers) {
            if (trigger->shouldActivate(gameMap, triggerCell)) {
                shouldActivate = true;
                break;
            }
        }
        
        // Si ningún trigger se activa, no activar el efecto
        if (!shouldActivate) return false;
        
        // Seleccionar objetivos usando el TargetSelector
        auto selectedTargets = targetSelector->selectTargets(gameMap, triggerCell);
        
        // Verificar si el efecto es aplicable con los objetivos seleccionados
        return effectImpl->isApplicable(gameMap, triggerCell, selectedTargets);
    }
    
    // Implementar la aplicación de efectos compuestos
    void apply(GameMap& map, MapCell* triggerCell, const std::list<MapCell*>& affectedCells) override {
        (void)affectedCells; // Suprimir warning de parámetro no usado
        
        // Seleccionar objetivos usando el TargetSelector
        auto selectedTargets = targetSelector->selectTargets(map, triggerCell);
        
        // Aplicar la implementación del efecto a los objetivos seleccionados
        effectImpl->apply(map, triggerCell, selectedTargets);
    }
    
    // Método adicional para aplicar efectos de estado de juego
    void applyToGameState(GameMap& map, MapCell* triggerCell, GameState* gameState) {
        // Solo aplicar si algún trigger está activo
        bool shouldActivate = false;
        for (auto& trigger : triggers) {
            if (trigger->shouldActivate(map, triggerCell)) {
                shouldActivate = true;
                break;
            }
        }
        
        if (shouldActivate) {
            effectImpl->applyToGameState(map, triggerCell, gameState);
        }
    }
    
    // Implementación de execute para cumplir con la interfaz Effect
    void execute(GameMap& gameMap, MapCell* triggerCell) override {
        // Seleccionar objetivos usando el TargetSelector
        auto selectedTargets = targetSelector->selectTargets(gameMap, triggerCell);
        // Verificar si el efecto es aplicable
        if (checkEffect(gameMap, triggerCell, selectedTargets)) {
            apply(gameMap, triggerCell, selectedTargets);
        }
    }
    
    // Getters para el nuevo sistema
    EffectImplPtr getEffectImpl() const { return effectImpl; }
    const std::vector<TriggerPtr>& getTriggers() const { return triggers; }
    TargetSelectorPtr getTargetSelector() const { return targetSelector; }
    
    // Operador de conversión a string (para depuración)
    operator std::string() const override {
        std::string result = "ComposedEffect [Impl: ";
        result += static_cast<std::string>(*effectImpl);
        result += ", Targets: ";
        result += static_cast<std::string>(*targetSelector);
        result += ", Triggers: ";
        for (const auto& trigger : triggers) {
            result += static_cast<std::string>(*trigger) + " ";
        }
        result += "]";
        return result;
    }
};
