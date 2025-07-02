#include "EffectLexer.hpp"
#include <cstring>

namespace lex {

    std::expected<EffectType, ParseError> EffectLexer::parseEffectTypeImpl(const char* input) {
        const char* ptr = input;
        
        switch (*ptr) {
            case 'a':
                goto state_a;
            case 'd':
                goto state_d;
            case 'h':
                goto state_h;
            case 'p':
                goto state_p;
            case 'r':
                goto state_r;
            case 't':
                goto state_t; // Nuevo: targeted effects
            default:
                return std::unexpected(ParseError::UNKNOWN_PREFIX);
        }

    state_a:
        ++ptr;
        if (!*ptr) return std::unexpected(ParseError::INCOMPLETE_INPUT);
        
        switch (*ptr) {
            case 'd':
                goto state_ad; // adjacency_buff
            case 't':
                goto state_at; // attack_*
            default:
                return std::unexpected(ParseError::MALFORMED_STRING);
        }

    state_ad:
        ++ptr;
        if (memcmp(ptr, "jacency_buff", 12) == 0 && ptr[12] == '\0') {
            return EffectType::ADJACENCY_BUFF;
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    state_at:
        ++ptr;
        if (memcmp(ptr, "tack_", 5) == 0) {
            ptr += 5;
            if (memcmp(ptr, "buff", 4) == 0 && ptr[4] == '\0') {
                return EffectType::ATTACK_BUFF;
            }
            if (memcmp(ptr, "debuff", 6) == 0 && ptr[6] == '\0') {
                return EffectType::ATTACK_DEBUFF;
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    state_d:
        ++ptr;
        if (memcmp(ptr, "amage", 5) == 0 && ptr[5] == '\0') {
            return EffectType::DAMAGE;
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    state_h:
        ++ptr;
        if (!*ptr) return std::unexpected(ParseError::INCOMPLETE_INPUT);
        
        // Check for "heal" first
        if (memcmp(ptr, "eal", 3) == 0) {
            ptr += 3;
            if (*ptr == '\0') return EffectType::HEAL;
            
            // Check for "health_buff" or "health_debuff"
            if (memcmp(ptr, "th_", 3) == 0) {
                ptr += 3;
                if (memcmp(ptr, "buff", 4) == 0 && ptr[4] == '\0') {
                    return EffectType::HEALTH_BUFF;
                }
                if (memcmp(ptr, "debuff", 6) == 0 && ptr[6] == '\0') {
                    return EffectType::HEALTH_DEBUFF;
                }
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    state_p:
        ++ptr;
        if (memcmp(ptr, "osition", 7) == 0) {
            ptr += 7;
            
            if (*ptr == '_') {
                ++ptr;
                if (memcmp(ptr, "enter_effect", 12) == 0 && ptr[12] == '\0') {
                    return EffectType::POSITION_ENTER_EFFECT;
                }
            }
            
            if (memcmp(ptr, "al_", 3) == 0) {
                ptr += 3;
                if (memcmp(ptr, "debuff", 6) == 0 && ptr[6] == '\0') {
                    return EffectType::POSITIONAL_DEBUFF;
                }
                if (memcmp(ptr, "trigger", 7) == 0 && ptr[7] == '\0') {
                    return EffectType::POSITIONAL_TRIGGER;
                }
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    state_r:
        ++ptr;
        if (memcmp(ptr, "ange_", 5) == 0) {
            ptr += 5;
            if (memcmp(ptr, "buff", 4) == 0 && ptr[4] == '\0') {
                return EffectType::RANGE_BUFF;
            }
            if (memcmp(ptr, "debuff", 6) == 0 && ptr[6] == '\0') {
                return EffectType::RANGE_DEBUFF;
            }
            if (memcmp(ptr, "effect", 6) == 0 && ptr[6] == '\0') {
                return EffectType::RANGE_EFFECT;
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    state_t:
        ++ptr;
        if (memcmp(ptr, "argeted_", 8) == 0) {
            ptr += 8;
            if (memcmp(ptr, "heal", 4) == 0 && ptr[4] == '\0') {
                return EffectType::TARGETED_HEAL;
            }
            if (memcmp(ptr, "damage", 6) == 0 && ptr[6] == '\0') {
                return EffectType::TARGETED_DAMAGE;
            }
            if (memcmp(ptr, "buff", 4) == 0 && ptr[4] == '\0') {
                return EffectType::TARGETED_BUFF;
            }
            if (memcmp(ptr, "debuff", 6) == 0 && ptr[6] == '\0') {
                return EffectType::TARGETED_DEBUFF;
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    } // Fin de parseEffectTypeImpl

    std::expected<AttackModifierEffect::TargetType, ParseError> EffectLexer::parseTargetTypeImpl(const char* input) {
        const char* ptr = input;
        
        switch (*ptr) {
            case 'a':
                goto target_a;
            case 's':
                goto target_s;
            default:
                return AttackModifierEffect::TargetType::SELF;
        }

    target_a:
        ++ptr;
        if (!*ptr) return AttackModifierEffect::TargetType::SELF;
        
        if (*ptr == 'd') {
            ++ptr;
            if (memcmp(ptr, "jacent", 6) == 0 && ptr[6] == '\0') {
                return AttackModifierEffect::TargetType::ADJACENT;
            }
        } else if (*ptr == 'l') {
            ++ptr;
            if (memcmp(ptr, "l_", 2) == 0) {
                ptr += 2;
                if (memcmp(ptr, "enemy", 5) == 0 && ptr[5] == '\0') {
                    return AttackModifierEffect::TargetType::ALL_ENEMY;
                }
                if (memcmp(ptr, "friendly", 8) == 0 && ptr[8] == '\0') {
                    return AttackModifierEffect::TargetType::ALL_FRIENDLY;
                }
            }
        }
        return AttackModifierEffect::TargetType::SELF;

    target_s:
        ++ptr;
        if (!*ptr) return AttackModifierEffect::TargetType::SELF;
        
        if (*ptr == 'e') {
            ++ptr;
            if (memcmp(ptr, "lf", 2) == 0 && ptr[2] == '\0') {
                return AttackModifierEffect::TargetType::SELF;
            }
        } else if (*ptr == 'p') {
            ++ptr;
            if (memcmp(ptr, "ecific_position", 15) == 0 && ptr[15] == '\0') {
                return AttackModifierEffect::TargetType::SPECIFIC_POSITION;
            }
        }
        return AttackModifierEffect::TargetType::SELF;
    }

    std::expected<GameMap::Adjacency, ParseError> EffectLexer::parseDirectionImpl(const char* input) {
        if (!input || !*input) return std::unexpected(ParseError::INCOMPLETE_INPUT);
        const char* ptr = input;

        switch (*ptr) {
            case 'u':
                goto dir_u;
            case 'd':
                goto dir_d;
            case 't':
                goto dir_t;
            case 'b':
                goto dir_b;
            default:
                return std::unexpected(ParseError::UNKNOWN_PREFIX);
        }

    dir_u:
        ++ptr;
        if (!*ptr) return std::unexpected(ParseError::INCOMPLETE_INPUT);
        if (*ptr == 'p') {
            ++ptr;
            if (*ptr == '\0') return GameMap::Adjacency::UP;
            if (*ptr == '_') {
                ++ptr;
                if (memcmp(ptr, "left", 4) == 0 && ptr[4] == '\0')
                    return GameMap::Adjacency::TOP_LEFT;
                if (memcmp(ptr, "right", 5) == 0 && ptr[5] == '\0')
                    return GameMap::Adjacency::TOP_RIGHT;
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    dir_d:
        ++ptr;
        if (!*ptr) return std::unexpected(ParseError::INCOMPLETE_INPUT);
        if (memcmp(ptr, "own", 3) == 0) {
            ptr += 3;
            if (*ptr == '\0') return GameMap::Adjacency::DOWN;
            if (*ptr == '_') {
                ++ptr;
                if (memcmp(ptr, "left", 4) == 0 && ptr[4] == '\0')
                    return GameMap::Adjacency::BOTTOM_LEFT;
                if (memcmp(ptr, "right", 5) == 0 && ptr[5] == '\0')
                    return GameMap::Adjacency::BOTTOM_RIGHT;
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    dir_t:
        ++ptr;
        if (memcmp(ptr, "op_", 3) == 0) {
            ptr += 3;
            if (memcmp(ptr, "left", 4) == 0 && ptr[4] == '\0')
                return GameMap::Adjacency::TOP_LEFT;
            if (memcmp(ptr, "right", 5) == 0 && ptr[5] == '\0')
                return GameMap::Adjacency::TOP_RIGHT;
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    dir_b:
        ++ptr;
        if (memcmp(ptr, "ottom_", 6) == 0) {
            ptr += 6;
            if (memcmp(ptr, "left", 4) == 0 && ptr[4] == '\0')
                return GameMap::Adjacency::BOTTOM_LEFT;
            if (memcmp(ptr, "right", 5) == 0 && ptr[5] == '\0')
                return GameMap::Adjacency::BOTTOM_RIGHT;
        }
        return std::unexpected(ParseError::MALFORMED_STRING);
    }

} // namespace lex
