# SD Game Engine 2XD

## ¿Qué hace el programa?

Este es un motor de juego de cartas estratégicas en 2D que permite crear y ejecutar partidas entre dos jugadores en un mapa hexagonal. El sistema incluye cartas con efectos personalizables, mecánicas de combate, movimiento y un sistema de efectos compuestos configurables mediante archivos JSON.

## Compilación y Ejecución

### Compilar
```bash
make
```

### Limpiar y recompilar
```bash
make clean && make
```

### Ejecutar
```bash
./main
```

El programa ejecuta automáticamente una suite de 9 pruebas que verifican todos los componentes del motor.

## Archivo de Configuración

El programa utiliza `game_config.conf` para configurar parámetros del juego:

```ini
# Configuración del mapa
map_width=5          # Ancho del mapa (default: 5)
map_height=6         # Alto del mapa (default: 6)

# Configuración de jugadores
max_hand_size=7              # Tamaño máximo de mano (default: 7)
initial_hand_size=5          # Cartas iniciales (default: 5)
max_actions_per_turn=3       # Acciones por turno (default: 3)

# Configuración de cartas
max_legends_per_deck=3       # Cartas legendarias máximas (default: 3)
```

Todos los valores son opcionales y tienen valores por defecto si no se especifican.

---

## Sistema de Cartas, Efectos y Mapa

### Sistema de Cartas

El motor maneja tres tipos principales de cartas:

- **Unidades**: Cartas que se colocan en el mapa con atributos de ataque, vida y movimiento
- **Hechizos**: Cartas de uso instantáneo con efectos específicos
- **Legendarias**: Cartas especiales con efectos únicos (limitadas por configuración)

Cada carta tiene:
- **ID único**: Para identificación en el sistema
- **Nombre y descripción**: Información visual
- **Costo de maná**: Recurso necesario para jugarla
- **Tipo**: Determina cómo interactúa con el juego
- **Efectos**: Lista de efectos que se ejecutan al usar la carta

### Sistema de Efectos

El motor utiliza un sistema modular de efectos compuestos:

#### Efectos Base
- **Efectos de targeting**: Seleccionan objetivos en el mapa
- **Efectos de modificación**: Cambian atributos de cartas
- **Efectos de invocación**: Crean nuevas cartas en el mapa
- **Efectos de daño/curación**: Modifican la vida de las unidades

#### Efectos Compuestos
Los efectos se pueden combinar para crear comportamientos complejos:
- **Secuenciales**: Ejecutan efectos uno tras otro
- **Condicionales**: Ejecutan efectos según condiciones del juego
- **Triggered**: Se activan por eventos específicos (entrada a posición, ataque, etc.)

#### Configuración de Efectos
Los efectos se definen en JSON con la siguiente estructura:
```json
{
  "type": "TipoEfecto",
  "targeting": {
    "type": "Adjacent|AllCards|SpecificPosition",
    "filter": "ALLIES_ONLY|ENEMIES_ONLY|ANY"
  },
  "params": {
    "damage": 2,
    "heal": 3,
    "stat_bonus": 1
  }
}
```

### Sistema de Mapa

#### Estructura del Mapa
- **Mapa hexagonal**: Grid de celdas hexagonales para gameplay estratégico
- **Celdas**: Cada posición puede contener una carta o estar vacía
- **Coordenadas**: Sistema (x,y) para posicionamiento preciso

#### Mecánicas de Movimiento
- **Adyacencia**: Las cartas pueden moverse a celdas adyacentes
- **Limitaciones**: Cada carta tiene un rango de movimiento específico
- **Bloqueo**: Las cartas no pueden moverse a celdas ocupadas

#### Sistema de Combate
- **Combate directo**: Las cartas pueden atacar a cartas enemigas adyacentes
- **Cálculo de daño**: Se considera ataque del atacante vs vida del defensor
- **Eliminación**: Las cartas con vida <= 0 se retiran del mapa

#### Targeting en Mapa
El sistema permite múltiples tipos de selección de objetivos:
- **Objetivos adyacentes**: Celdas vecinas a una posición
- **Objetivos direccionales**: En una dirección específica
- **Objetivos por posición**: Coordenadas exactas
- **Todos los objetivos**: Todas las cartas en el mapa
- **Filtros**: Solo aliados, solo enemigos, o cualquier carta

Este sistema modular permite crear mecánicas de juego complejas y variadas mediante la combinación de efectos simples y sistemas de targeting flexibles.