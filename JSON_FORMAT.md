# Formato JSON de Mazos - SD Game Engine 2XD

Este documento describe el formato JSON utilizado para definir mazos de cartas en el motor de juego.

## Estructura General

El archivo JSON principal debe contener un array de mazos bajo la clave `"decks"`:

```json
{
  "decks": [
    {
      "name": "Nombre del Mazo",
      "cards": [
        // Array de cartas
      ]
    }
  ]
}
```

---

## Estructura de Mazo

### Campos del Mazo

| Campo | Tipo | Requerido | Descripción |
|-------|------|-----------|-------------|
| `name` | string | Sí | Nombre descriptivo del mazo |
| `cards` | array | Sí | Array de objetos carta |

### Ejemplo:
```json
{
  "name": "Team Alpha - Deck de Guerreros",
  "cards": [...]
}
```

---

## Estructura de Carta

### Campos Básicos de Carta

| Campo | Tipo | Requerido | Descripción |
|-------|------|-----------|-------------|
| `id` | integer | Sí | Identificador único de la carta |
| `name` | string | Sí | Nombre de la carta |
| `description` | string | Sí | Descripción de la carta |
| `cost` | integer | Sí | Costo de maná para jugar la carta |
| `type` | string | Sí | Tipo de carta: `"unit"`, `"spell"`, `"legend"` |
| `effects` | array | Sí | Array de efectos (puede estar vacío) |

### Campos Específicos para Unidades
Solo se aplican a cartas de tipo `"unit"` y `"legend"`:

| Campo | Tipo | Requerido | Descripción |
|-------|------|-----------|-------------|
| `attack` | integer | Sí | Valor de ataque de la unidad |
| `health` | integer | Sí | Puntos de vida de la unidad |
| `speed` | integer | Sí | Velocidad de movimiento |
| `range` | integer | Sí | Alcance de ataque |

### Ejemplo de Carta Básica:
```json
{
  "id": 1,
  "name": "Rey Guerrero",
  "description": "Leyenda que fortalece a los aliados adyacentes al ser invocado",
  "cost": 0,
  "type": "legend",
  "attack": 4,
  "health": 8,
  "speed": 2,
  "range": 1,
  "effects": [...]
}
```

---

## Estructura de Efectos

Los efectos se definen como objetos JSON con los siguientes campos:

### Campos Comunes de Efectos

| Campo | Tipo | Requerido | Descripción |
|-------|------|-----------|-------------|
| `type` | string | Sí | Tipo de efecto (ver tipos disponibles) |
| `target_type` | string | Sí | Tipo de targeting (ver tipos disponibles) |
| `filter` | string | Sí | Filtro de objetivos (ver filtros disponibles) |
| `trigger` | string | Sí | Cuándo se activa el efecto (ver triggers disponibles) |
| `duration` | integer | Sí | Duración del efecto (-1 = permanente, 0 = instantáneo, >0 = turnos) |

### Campos Opcionales de Efectos

| Campo | Tipo | Requerido | Descripción |
|-------|------|-----------|-------------|
| `directions` | array | No | Array de direcciones para targeting adyacente |
| `value` | integer | Depende | Valor numérico del efecto (daño, curación, etc.) |
| `attribute` | string | Depende | Atributo a modificar (para `attribute_modifier`) |

---

## Tipos de Efectos

### `"attribute_modifier"`
Modifica atributos de las cartas objetivo.

**Campos requeridos adicionales:**
- `attribute`: `"attack"`, `"health"`, `"speed"`, `"range"`
- `value`: Valor a sumar/restar (puede ser negativo)

```json
{
  "type": "attribute_modifier",
  "target_type": "adjacent",
  "filter": "allies_only",
  "trigger": "on_play",
  "attribute": "attack",
  "value": 2,
  "duration": 1
}
```

### `"direct_damage"`
Causa daño directo a los objetivos.

**Campos requeridos adicionales:**
- `value`: Cantidad de daño a infligir

```json
{
  "type": "direct_damage",
  "target_type": "adjacent",
  "filter": "enemies_only",
  "trigger": "turn_start",
  "value": 1,
  "duration": -1
}
```

### `"heal"`
Cura vida a los objetivos.

**Campos requeridos adicionales:**
- `value`: Cantidad de vida a restaurar

```json
{
  "type": "heal",
  "target_type": "all_allies",
  "filter": "allies_only",
  "trigger": "on_play",
  "value": 2,
  "duration": 0
}
```

---

## Tipos de Targeting

### `"adjacent"`
Selecciona cartas adyacentes a la carta fuente.

**Campos opcionales:**
- `directions`: Array específico de direcciones

### `"all_allies"`
Selecciona todas las cartas aliadas en el mapa.

### `"all_enemies"`
Selecciona todas las cartas enemigas en el mapa.

### `"all_cards"`
Selecciona todas las cartas en el mapa.

### `"self"`
Selecciona solo la carta que ejecuta el efecto.

### `"specific_position"`
Selecciona una posición específica (requiere configuración adicional).

---

## Filtros de Objetivos

| Filtro | Descripción |
|--------|-------------|
| `"allies_only"` | Solo cartas del mismo jugador |
| `"enemies_only"` | Solo cartas del jugador oponente |
| `"any"` | Cualquier carta (aliada o enemiga) |

---

## Triggers (Activadores)

| Trigger | Descripción |
|---------|-------------|
| `"on_play"` | Se activa cuando la carta se juega |
| `"on_death"` | Se activa cuando la carta muere |
| `"on_kill"` | Se activa cuando la carta mata a otra |
| `"turn_start"` | Se activa al inicio de cada turno |
| `"turn_end"` | Se activa al final de cada turno |
| `"on_attack"` | Se activa cuando la carta ataca |
| `"on_defend"` | Se activa cuando la carta es atacada |

---

## Direcciones para Targeting Adyacente

Para efectos con `target_type: "adjacent"`, se puede especificar direcciones:

```json
"directions": ["up", "down", "top_left", "top_right", "bottom_left", "bottom_right"]
```

**Direcciones disponibles:**
- `"up"`: Arriba
- `"down"`: Abajo  
- `"top_left"`: Arriba-izquierda
- `"top_right"`: Arriba-derecha
- `"bottom_left"`: Abajo-izquierda
- `"bottom_right"`: Abajo-derecha

Si se omite `directions`, se consideran todas las direcciones adyacentes.

---

## Valores de Duración

| Valor | Significado |
|-------|-------------|
| `-1` | Efecto permanente (hasta que la carta muera o se remueva) |
| `0` | Efecto instantáneo (se aplica una vez inmediatamente) |
| `> 0` | Efecto temporal (dura X turnos) |

---

## Ejemplo Completo

```json
{
  "decks": [
    {
      "name": "Mazo de Ejemplo",
      "cards": [
        {
          "id": 1,
          "name": "Guerrero Básico",
          "description": "Unidad básica de combate",
          "cost": 2,
          "type": "unit",
          "attack": 2,
          "health": 3,
          "speed": 2,
          "range": 1,
          "effects": []
        },
        {
          "id": 2,
          "name": "Curandero",
          "description": "Cura aliados adyacentes al ser jugado",
          "cost": 3,
          "type": "unit",
          "attack": 1,
          "health": 4,
          "speed": 1,
          "range": 1,
          "effects": [
            {
              "type": "heal",
              "target_type": "adjacent",
              "filter": "allies_only",
              "trigger": "on_play",
              "directions": ["up", "down", "top_left", "top_right"],
              "value": 2,
              "duration": 0
            }
          ]
        },
        {
          "id": 3,
          "name": "Hechizo de Rayo",
          "description": "Hace daño directo a un enemigo",
          "cost": 1,
          "type": "spell",
          "effects": [
            {
              "type": "direct_damage",
              "target_type": "all_enemies",
              "filter": "enemies_only",
              "trigger": "on_play",
              "value": 3,
              "duration": 0
            }
          ]
        }
      ]
    }
  ]
}
```

---

## Validaciones

### Validaciones de Carta
- `id` debe ser único dentro del mazo
- `cost` debe ser >= 0
- Para unidades: `attack`, `health`, `speed`, `range` deben ser > 0
- `type` debe ser uno de los valores válidos

### Validaciones de Efecto
- Combinaciones de `type` y campos requeridos deben ser consistentes
- `value` debe ser apropiado para el tipo de efecto
- `duration` debe ser >= -1
- `directions` solo es válido para targeting adyacente

### Validaciones de Consistencia
- Hechizos no deben tener atributos de unidad
- Unidades deben tener todos los atributos requeridos
- Efectos deben tener campos apropiados para su tipo
