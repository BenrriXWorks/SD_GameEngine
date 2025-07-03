# Documentación de Clases - SD Game Engine 2XD

## Módulo API (Principal)

### `GameAPI`
**Propósito**: Interfaz principal para todas las operaciones del juego. Esta es la clase más importante ya que actúa como punto de entrada único para todas las interacciones con el motor.

**Entradas Generales**:
- Archivos de configuración (mazos JSON, configuración .conf)
- Comandos de juego (mover carta, atacar, jugar carta, terminar turno)
- Parámetros de inicialización del juego

**Salidas Generales**:
- Información del estado del juego (`GameInfo`)
- Información de jugadores (`PlayerInfo`)
- Resultados de acciones (códigos de éxito/fallo con detalles)

### Métodos de Inicialización

#### `initializeGameWithConfig(deckFilePath, config)`
**Propósito**: Inicializa el juego con archivos de configuración.
- **Parámetros**:
  - `const std::string& deckFilePath`: Ruta al archivo JSON con mazos
  - `const GameConfig& config`: Objeto de configuración del juego
- **Retorna**: `bool` - true si la inicialización fue exitosa
- **Uso**: Método principal para inicializar el motor con configuración personalizada

### Métodos de Consulta de Estado

#### `getGameInfo()`
**Propósito**: Obtiene información completa del estado actual del juego.
- **Parámetros**: Ninguno
- **Retorna**: `GameInfo` - Estructura con toda la información del juego
- **Contenido del GameInfo**:
  - `turn`: Número de turno actual
  - `currentPlayer`: ID del jugador actual
  - `isGameOver`: Si el juego ha terminado
  - `winner`: Ganador (si aplica)
  - `player0`, `player1`: Información de ambos jugadores
  - `mapCells`: Estado de todas las celdas del mapa

#### `getPlayerInfo(playerId)`
**Propósito**: Obtiene información específica de un jugador.
- **Parámetros**:
  - `PlayerId playerId`: ID del jugador (0 o 1)
- **Retorna**: `PlayerInfo` - Información detallada del jugador
- **Contenido del PlayerInfo**:
  - `id`: ID del jugador
  - `team`: Equipo (TEAM_A o TEAM_B)
  - `isAlive`: Si el jugador está vivo
  - `actionsLeft`: Acciones restantes en el turno
  - `handSize`: Número de cartas en mano
  - `hasLegend`: Si tiene leyenda activa
  - `legendPosition`: Posición de la leyenda en el mapa

#### `getCellInfo(x, y)`
**Propósito**: Obtiene información de una celda específica del mapa.
- **Parámetros**:
  - `uint8_t x`: Coordenada X (0-4)
  - `uint8_t y`: Coordenada Y (0-5)
- **Retorna**: `std::optional<CellInfo>` - Información de la celda o nullopt si es inválida

### Métodos de Acción de Juego

#### `playCard(playerId, cardIndex, x, y)`
**Propósito**: Juega una carta desde la mano del jugador en una posición específica.
- **Parámetros**:
  - `PlayerId playerId`: ID del jugador que juega la carta
  - `size_t cardIndex`: Índice de la carta en la mano (0-6)
  - `uint8_t x`: Coordenada X donde colocar la carta
  - `uint8_t y`: Coordenada Y donde colocar la carta
- **Retorna**: `ActionResult` - Resultado de la acción
- **Validaciones**: Verifica turno, acciones disponibles, carta válida, posición legal

#### `moveCard(playerId, fromX, fromY, toX, toY)`
**Propósito**: Mueve una carta de una posición a otra en el mapa.
- **Parámetros**:
  - `PlayerId playerId`: ID del jugador dueño de la carta
  - `uint8_t fromX`: Coordenada X origen
  - `uint8_t fromY`: Coordenada Y origen
  - `uint8_t toX`: Coordenada X destino
  - `uint8_t toY`: Coordenada Y destino
- **Retorna**: `ActionResult` - Resultado del movimiento
- **Validaciones**: Verifica propiedad de carta, rango de movimiento, destino válido

#### `attackCard(playerId, fromX, fromY, toX, toY)`
**Propósito**: Realiza un ataque desde una carta hacia un objetivo.
- **Parámetros**:
  - `PlayerId playerId`: ID del jugador atacante
  - `uint8_t fromX`: Coordenada X de la carta atacante
  - `uint8_t fromY`: Coordenada Y de la carta atacante
  - `uint8_t toX`: Coordenada X del objetivo
  - `uint8_t toY`: Coordenada Y del objetivo
- **Retorna**: `ActionResult` - Resultado del ataque
- **Validaciones**: Verifica rango de ataque, objetivo válido, acciones disponibles

#### `castSpell(playerId, cardIndex)`
**Propósito**: Lanza un hechizo desde la mano del jugador.
- **Parámetros**:
  - `PlayerId playerId`: ID del jugador que lanza el hechizo
  - `size_t cardIndex`: Índice del hechizo en la mano
- **Retorna**: `ActionResult` - Resultado del lanzamiento
- **Validaciones**: Verifica que la carta es un hechizo, acciones disponibles

#### `endTurn()`
**Propósito**: Termina el turno del jugador actual.
- **Parámetros**: Ninguno
- **Retorna**: `ActionResult` - Siempre SUCCESS si el juego no ha terminado
- **Efectos**: Cambia jugador activo, resetea acciones, ejecuta efectos de fin de turno

### Métodos de Consulta de Movimientos Válidos

#### `getValidPlayPositions(playerId)`
**Propósito**: Obtiene todas las posiciones válidas donde el jugador puede colocar cartas.
- **Parámetros**:
  - `PlayerId playerId`: ID del jugador
- **Retorna**: `std::vector<std::pair<uint8_t, uint8_t>>` - Lista de coordenadas válidas

#### `getValidMovePositions(x, y)`
**Propósito**: Obtiene posiciones válidas donde se puede mover una carta específica.
- **Parámetros**:
  - `uint8_t x`: Coordenada X de la carta
  - `uint8_t y`: Coordenada Y de la carta
- **Retorna**: `std::vector<std::pair<uint8_t, uint8_t>>` - Destinos válidos

#### `getValidAttackPositions(x, y)`
**Propósito**: Obtiene objetivos válidos que puede atacar una carta específica.
- **Parámetros**:
  - `uint8_t x`: Coordenada X de la carta atacante
  - `uint8_t y`: Coordenada Y de la carta atacante
- **Retorna**: `std::vector<std::pair<uint8_t, uint8_t>>` - Objetivos válidos

### Códigos de Resultado (`ActionResult`)
- `SUCCESS`: Acción ejecutada exitosamente
- `INVALID_PLAYER`: Jugador inválido o no es su turno
- `INVALID_POSITION`: Posición fuera del mapa o inválida
- `INVALID_CARD`: Carta no existe o no válida para la acción
- `NOT_ENOUGH_ACTIONS`: Sin acciones restantes en el turno
- `POSITION_OCCUPIED`: Destino ocupado por otra carta
- `CANNOT_ATTACK_TARGET`: Objetivo no válido para ataque
- `OUT_OF_RANGE`: Acción fuera del rango permitido
- `GAME_OVER`: El juego ha terminado
- `INVALID_MOVE`: Movimiento no permitido

---

## Arquitectura General

El motor está organizado en varios módulos principales que manejan diferentes aspectos del juego:

- **API**: Interfaz principal para interactuar con el motor
- **Cartas**: Sistema de manejo de cartas y carga desde archivos
- **Efectos**: Sistema modular de efectos y su ejecución
- **Juego**: Estado del juego, mapa y mecánicas core
- **Targeting**: Sistema de selección de objetivos
- **Configuración**: Manejo de parámetros del juego

---

## Módulo de Cartas

### `Card` (Clase Base)
**Propósito**: Clase base abstracta para todas las cartas del juego.

**Entradas**: 
- ID, nombre, descripción, costo de maná, tipo de carta

**Salidas**:
- Información de la carta
- Conversión a string para debugging

**Subclases**:
- `Unit`: Cartas que se colocan en el mapa
- `Spell`: Cartas de efecto instantáneo

### `CardLoader`
**Propósito**: Carga y procesa definiciones de cartas desde archivos JSON.

**Entradas**:
- Archivos JSON con definiciones de mazos y cartas
- Configuraciones de efectos

**Salidas**:
- Vector de `CardPtr` listos para usar
- Estructuras `DeckConfig` con información procesada

**Métodos Principales**:
- `loadDecksFromFile()`: Carga mazos desde archivo JSON
- `createCardsFromConfig()`: Convierte configuración JSON a objetos Card

---

## Módulo de Efectos

### `Effect` (Clase Base)
**Propósito**: Clase base para todos los efectos del juego.

**Entradas**:
- Estado del juego (`GameState`)
- Parámetros específicos del efecto

**Salidas**:
- Booleano indicando éxito/fallo de la ejecución
- Modificaciones al estado del juego

**Métodos Virtuales**:
- `checkEffect()`: Verifica si el efecto puede ejecutarse
- `apply()`: Aplica el efecto al juego
- `execute()`: Ejecuta el efecto completo
- `operator std::string()`: Convierte a string para debugging

### `ComposedEffect`
**Propósito**: Contenedor de múltiples efectos que se ejecutan secuencialmente.

**Entradas**:
- Lista de `EffectPtr` a ejecutar
- Estado del juego

**Salidas**:
- Resultado combinado de todos los efectos
- Información de cuáles efectos se ejecutaron exitosamente

### `EffectStack`
**Propósito**: Pila de efectos que maneja el orden de ejecución y efectos triggered.

**Entradas**:
- Efectos a añadir a la pila
- Triggers del juego

**Salidas**:
- Ejecución ordenada de efectos
- Manejo de efectos en cadena

### `ComposedEffectFactory`
**Propósito**: Factory para crear efectos compuestos desde configuración JSON.

**Entradas**:
- Objetos JSON con definición de efectos
- Carta fuente y propietario

**Salidas**:
- `ComposedEffectPtr` listo para usar

---

## Módulo de Targeting

### `TargetSelector` (Clase Base)
**Propósito**: Clase base para seleccionar objetivos en el mapa.

**Entradas**:
- `GameMap` actual
- Celda que dispara el efecto (`triggerCell`)

**Salidas**:
- Lista de `MapCell*` que son objetivos válidos

### Implementaciones de TargetSelector

#### `AllCardsTargetSelector`
**Propósito**: Selecciona todas las cartas en el mapa con filtros opcionales.
- **Filtros**: ALLIES_ONLY, ENEMIES_ONLY, ANY

#### `AdjacentTargetSelector`
**Propósito**: Selecciona cartas adyacentes a una posición específica.
- **Entrada adicional**: Posición de referencia

#### `SpecificPositionTargetSelector`
**Propósito**: Selecciona una posición específica en el mapa.
- **Entrada adicional**: Coordenadas (x, y)

#### `DirectionalTargetSelector`
**Propósito**: Selecciona objetivos en una dirección específica.
- **Entrada adicional**: Dirección (`GameMap::Adjacency`)

#### `GameStateTargetSelector`
**Propósito**: Para efectos que no requieren targeting específico.
- **Salida**: Lista vacía (no selecciona celdas)

### `TargetSelectorFactory`
**Propósito**: Factory para crear selectores de targeting desde configuración.

**Entradas**:
- `ConfigLexer::TargetType` (tipo de targeting)
- Carta fuente, propietario, filtros
- Parámetros específicos (posición, dirección)

**Salidas**:
- `TargetSelectorPtr` configurado apropiadamente

---

## Módulo de Juego

### `GameState`
**Propósito**: Mantiene y gestiona todo el estado del juego.

**Entradas**:
- Configuración del juego
- Comandos de modificación de estado

**Salidas**:
- Estado actual del juego
- Validación de movimientos legales

**Componentes**:
- Mapa del juego (`GameMap`)
- Estado de jugadores
- Información de turnos
- Pila de efectos

### `GameMap`
**Propósito**: Representa el mapa hexagonal y maneja posicionamiento.

**Entradas**:
- Dimensiones del mapa (ancho, alto)
- Comandos de colocación/movimiento de cartas

**Salidas**:
- Información de celdas específicas
- Validación de posiciones y movimientos
- Cálculo de adyacencias

**Métodos Principales**:
- `at(x, y)`: Obtiene celda en posición específica
- `getAdjacent()`: Obtiene celdas adyacentes
- `isValidPosition()`: Valida coordenadas
- `placeCard()`: Coloca carta en posición
- `removeCard()`: Remueve carta de posición

### `MapCell`
**Propósito**: Representa una celda individual del mapa.

**Entradas**:
- Coordenadas (x, y)
- Carta a colocar (opcional)

**Salidas**:
- Información de ocupación
- Propietario de la carta (si existe)

---

## Módulo de Configuración

### `GameConfig`
**Propósito**: Maneja la configuración del juego desde archivos.

**Entradas**:
- Archivo de configuración `.conf`
- Valores por defecto

**Salidas**:
- Valores de configuración tipados
- Validación de parámetros

**Métodos Principales**:
- `getInt()`: Obtiene valor entero con default
- `getString()`: Obtiene valor string con default
- `getBool()`: Obtiene valor booleano con default

---

## Módulo de Utilidades

### `Types.hpp`
**Propósito**: Define tipos comunes utilizados en todo el sistema.

**Tipos Principales**:
- `PlayerId`: Identificador de jugador (unsigned)
- `CardPtr`: Smart pointer a Card
- `EffectPtr`: Smart pointer a Effect
- `TargetSelectorPtr`: Smart pointer a TargetSelector

### `StrColors.hpp`
**Propósito**: Constantes para colorear salida de consola.

**Salidas**:
- Códigos ANSI para colores en terminal

---

## Flujo de Datos Típico

1. **Inicialización**:
   - `GameAPI` lee configuración y mazos
   - `CardLoader` procesa JSON y crea cartas
   - `GameState` inicializa mapa y jugadores

2. **Ejecución de Turno**:
   - `GameAPI` recibe comando del jugador
   - `GameState` valida la acción
   - `EffectStack` procesa efectos resultantes
   - `TargetSelector` determina objetivos afectados

3. **Resolución de Efectos**:
   - `ComposedEffect` ejecuta efectos secuencialmente
   - `GameMap` se actualiza con cambios
   - `GameState` verifica condiciones de victoria

Este diseño modular permite extensibilidad fácil y separación clara de responsabilidades entre componentes.
