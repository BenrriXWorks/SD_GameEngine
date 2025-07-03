#include "EffectLexer.hpp"
#include <cstring>

namespace lex {

    std::expected<ConfigLexer::EffectType, ParseError> EffectLexer::parseEffectTypeImpl(const char* input) {
        const char* ptr = input;
        
        switch (*ptr) {
            case 'a':
                goto state_a;
            case 'd':
                goto state_d;
            case 'h':
                goto state_h;
            case 'r':
                goto state_r;
            case 'e':
                goto state_e;
            default:
                return std::unexpected(ParseError::UNKNOWN_PREFIX);
        }

    state_a:
        ++ptr;
        if (!*ptr) return std::unexpected(ParseError::INCOMPLETE_INPUT);
        
        if (memcmp(ptr, "ttribute_modifier", 17) == 0 && ptr[17] == '\0') {
            return ConfigLexer::EffectType::ATTRIBUTE_MODIFIER;
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    state_d:
        ++ptr;
        if (memcmp(ptr, "irect_damage", 12) == 0 && ptr[12] == '\0') {
            return ConfigLexer::EffectType::DIRECT_DAMAGE;
        }
        if (memcmp(ptr, "raw_cards", 9) == 0 && ptr[9] == '\0') {
            return ConfigLexer::EffectType::DRAW_CARDS;
        }
        if (memcmp(ptr, "estroy", 6) == 0 && ptr[6] == '\0') {
            return ConfigLexer::EffectType::DESTROY;
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    state_h:
        ++ptr;
        if (memcmp(ptr, "eal", 3) == 0 && ptr[3] == '\0') {
            return ConfigLexer::EffectType::HEAL;
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    state_r:
        ++ptr;
        if (memcmp(ptr, "eflect_damage", 13) == 0 && ptr[13] == '\0') {
            return ConfigLexer::EffectType::REFLECT_DAMAGE;
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    state_e:
        // This is a stub for future effect types starting with 'e'
        return std::unexpected(ParseError::MALFORMED_STRING);
    } // Fin de parseEffectTypeImpl

    std::expected<ConfigLexer::TargetType, ParseError> EffectLexer::parseTargetTypeImpl(const char* input) {
        const char* ptr = input;
        
        switch (*ptr) {
            case 'a':
                goto target_a;
            case 's':
                goto target_s;
            case 'g':
                goto target_g;
            default:
                return std::unexpected(ParseError::UNKNOWN_PREFIX);
        }

    target_a:
        ++ptr;
        if (!*ptr) return std::unexpected(ParseError::INCOMPLETE_INPUT);
        
        if (memcmp(ptr, "djacent", 7) == 0 && ptr[7] == '\0') {
            return ConfigLexer::TargetType::ADJACENT;
        }
        if (memcmp(ptr, "ttack_target", 12) == 0 && ptr[12] == '\0') {
            return ConfigLexer::TargetType::ATTACK_TARGET;
        }
        if (memcmp(ptr, "ttacker", 7) == 0 && ptr[7] == '\0') {
            return ConfigLexer::TargetType::ATTACKER;
        }
        if (*ptr == 'l') {
            ++ptr;
            if (memcmp(ptr, "l_", 2) == 0) {
                ptr += 2;
                if (memcmp(ptr, "allies", 6) == 0 && ptr[6] == '\0') {
                    return ConfigLexer::TargetType::ALL_ALLIES;
                }
                if (memcmp(ptr, "enemies", 7) == 0 && ptr[7] == '\0') {
                    return ConfigLexer::TargetType::ALL_ENEMIES;
                }
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    target_s:
        ++ptr;
        if (!*ptr) return std::unexpected(ParseError::INCOMPLETE_INPUT);
        
        if (*ptr == 'e') {
            ++ptr;
            if (memcmp(ptr, "lf", 2) == 0 && ptr[2] == '\0') {
                return ConfigLexer::TargetType::SELF;
            }
        } else if (*ptr == 'p') {
            ++ptr;
            if (memcmp(ptr, "ecific_position", 15) == 0 && ptr[15] == '\0') {
                return ConfigLexer::TargetType::SPECIFIC_POSITION;
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    target_g:
        ++ptr;
        if (memcmp(ptr, "ame_state", 9) == 0 && ptr[9] == '\0') {
            return ConfigLexer::TargetType::GAME_STATE;
        }
        return std::unexpected(ParseError::MALFORMED_STRING);
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

    std::expected<ConfigLexer::TriggerType, ParseError> EffectLexer::parseTriggerTypeImpl(const char* input) {
        const char* ptr = input;
        
        switch (*ptr) {
            case 'o':
                goto trigger_o;
            case 't':
                goto trigger_t;
            default:
                return std::unexpected(ParseError::UNKNOWN_PREFIX);
        }

    trigger_o:
        ++ptr;
        if (!*ptr) return std::unexpected(ParseError::INCOMPLETE_INPUT);
        
        if (*ptr == 'n') {
            ++ptr;
            if (*ptr == '_') {
                ++ptr;
                switch (*ptr) {
                    case 'p':
                        ++ptr;
                        if (memcmp(ptr, "lay", 3) == 0 && ptr[3] == '\0') {
                            return ConfigLexer::TriggerType::ON_PLAY;
                        }
                        break;
                    case 'a':
                        ++ptr;
                        if (memcmp(ptr, "ttack", 5) == 0 && ptr[5] == '\0') {
                            return ConfigLexer::TriggerType::ON_ATTACK;
                        }
                        if (memcmp(ptr, "ttacked", 7) == 0 && ptr[7] == '\0') {
                            return ConfigLexer::TriggerType::ON_ATTACKED;
                        }
                        break;
                    case 'k':
                        ++ptr;
                        if (memcmp(ptr, "ill", 3) == 0 && ptr[3] == '\0') {
                            return ConfigLexer::TriggerType::ON_KILL;
                        }
                        break;
                    case 'd':
                        ++ptr;
                        if (memcmp(ptr, "eath", 4) == 0 && ptr[4] == '\0') {
                            return ConfigLexer::TriggerType::ON_DEATH;
                        }
                        break;
                }
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);

    trigger_t:
        ++ptr;
        if (!*ptr) return std::unexpected(ParseError::INCOMPLETE_INPUT);
        
        if (memcmp(ptr, "urn_", 4) == 0) {
            ptr += 4;
            if (memcmp(ptr, "start", 5) == 0 && ptr[5] == '\0') {
                return ConfigLexer::TriggerType::TURN_START;
            }
            if (memcmp(ptr, "end", 3) == 0 && ptr[3] == '\0') {
                return ConfigLexer::TriggerType::TURN_END;
            }
        }
        return std::unexpected(ParseError::MALFORMED_STRING);
    }

} // namespace lex
