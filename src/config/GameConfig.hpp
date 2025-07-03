#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include "../../libs/flat_hash_map.hpp"
#include <print>

class GameConfig {
private:
    ska::flat_hash_map<std::string, std::string> config;
    
public:
    // Cargar configuración desde archivo
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::println("Error: No se pudo abrir el archivo de configuración: {}", filename);
            return false;
        }
        
        std::string line;
        size_t lineNumber = 0;
        
        while (std::getline(file, line)) {
            lineNumber++;
            
            // Eliminar espacios al inicio y final
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
            // Ignorar líneas vacías y comentarios
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            // Buscar el separador '='
            size_t equalPos = line.find('=');
            if (equalPos == std::string::npos) {
                std::println("Warning: Línea {} ignorada - formato inválido: {}", lineNumber, line);
                continue;
            }
            
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            
            // Eliminar espacios alrededor de la clave y valor
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            if (!key.empty()) {
                config[key] = value;
                std::println("Config cargada: {} = {}", key, value);
            }
        }
        
        std::println("Configuración cargada exitosamente: {} entradas", config.size());
        return true;
    }
    
    // Obtener valor como string
    std::string getString(const std::string& key, const std::string& defaultValue = "") const {
        auto it = config.find(key);
        return (it != config.end()) ? it->second : defaultValue;
    }
    
    // Obtener valor como entero
    int getInt(const std::string& key, int defaultValue = 0) const {
        auto it = config.find(key);
        if (it != config.end()) {
            try {
                return std::stoi(it->second);
            } catch (const std::exception&) {
                std::println("Warning: No se pudo convertir '{}' a entero para la clave '{}'", it->second, key);
            }
        }
        return defaultValue;
    }
    
    // Obtener valor como boolean
    bool getBool(const std::string& key, bool defaultValue = false) const {
        auto it = config.find(key);
        if (it != config.end()) {
            std::string value = it->second;
            // Convertir a minúsculas
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            return (value == "true" || value == "1" || value == "yes" || value == "on");
        }
        return defaultValue;
    }
    
    // Obtener valor como float
    float getFloat(const std::string& key, float defaultValue = 0.0f) const {
        auto it = config.find(key);
        if (it != config.end()) {
            try {
                return std::stof(it->second);
            } catch (const std::exception&) {
                std::println("Warning: No se pudo convertir '{}' a float para la clave '{}'", it->second, key);
            }
        }
        return defaultValue;
    }
    
    // Obtener valor como uint8_t
    uint8_t getUInt8(const std::string& key, uint8_t defaultValue = 0) const {
        int value = getInt(key, defaultValue);
        if (value < 0 || value > 255) {
            std::println("Warning: Valor '{}' fuera de rango para uint8_t en clave '{}', usando {}", value, key, defaultValue);
            return defaultValue;
        }
        return static_cast<uint8_t>(value);
    }
    
    // Verificar si existe una clave
    bool hasKey(const std::string& key) const {
        return config.find(key) != config.end();
    }
    
    // Obtener todas las configuraciones (para depuración)
    const ska::flat_hash_map<std::string, std::string>& getAllConfig() const {
        return config;
    }
    
    // Establecer valor (útil para tests)
    void set(const std::string& key, const std::string& value) {
        config[key] = value;
    }
    
    void set(const std::string& key, int value) {
        config[key] = std::to_string(value);
    }
    
    void set(const std::string& key, bool value) {
        config[key] = value ? "true" : "false";
    }
    
    void set(const std::string& key, float value) {
        config[key] = std::to_string(value);
    }
    
    // Método para imprimir toda la configuración
    void printAll() const {
        std::println("=== CONFIGURACIÓN DEL JUEGO ===");
        for (const auto& [key, value] : config) {
            std::println("{} = {}", key, value);
        }
        std::println("===============================");
    }
};
