# Documentación de Clases - SD Game Engine 2XD

## Arquitectura General

El motor está organizado en varios módulos principales que manejan diferentes aspectos del juego:

- **API**: Interfaz principal para interactuar con el motor
- **Cartas**: Sistema de manejo de cartas y carga desde archivos
- **Efectos**: Sistema modular de efectos y su ejecución
- **Juego**: Estado del juego, mapa y mecánicas core
- **Targeting**: Sistema de selección de objetivos
- **Configuración**: Manejo de parámetros del juego

---

## Módulo API

### `GameAPI`
**Propósito**: Interfaz principal para todas las operaciones del juego.

**Entradas**:
- Archivos de configuración (mazos, config)
- Comandos de juego (mover carta, atacar, jugar carta, terminar turno)
- Parámetros de inicialización

**Salidas**:
- Información del estado del juego (`GameInfo`)
- Información de jugadores (`PlayerInfo`)
- Resultados de acciones (éxito/fallo con detalles)

**Métodos Principales**:
- `initializeGameWithConfig()`: Inicializa el juego con archivos de configuración
- `getGameInfo()`: Obtiene el estado actual del juego
- `getPlayerInfo()`: Obtiene información de un jugador específico
- `playCard()`: Juega una carta desde la mano
- `moveCard()`: Mueve una carta en el mapa
- `attackCard()`: Realiza un ataque entre cartas
- `endTurn()`: Termina el turno actual

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
