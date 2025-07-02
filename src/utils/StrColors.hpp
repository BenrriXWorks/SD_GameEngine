#pragma once
#include <string>

namespace StrColors { 
    static constexpr std::string BLACK = "\x1B[30m";
    static constexpr std::string RED = "\x1B[31m";
    static constexpr std::string GREEN = "\x1B[32m";
    static constexpr std::string YELLOW = "\x1B[33m";
    static constexpr std::string BLUE = "\x1B[34m";
    static constexpr std::string MAGENTA = "\x1B[35m";
    static constexpr std::string CYAN = "\x1B[36m";
    static constexpr std::string WHITE = "\x1B[37m";
    
    // Colores brillantes (90-97)
    static constexpr std::string BRIGHT_BLACK = "\x1B[90m";   // Gris
    static constexpr std::string BRIGHT_RED = "\x1B[91m";
    static constexpr std::string BRIGHT_GREEN = "\x1B[92m";
    static constexpr std::string BRIGHT_YELLOW = "\x1B[93m";
    static constexpr std::string BRIGHT_BLUE = "\x1B[94m";
    static constexpr std::string BRIGHT_MAGENTA = "\x1B[95m";
    static constexpr std::string BRIGHT_CYAN = "\x1B[96m";
    static constexpr std::string BRIGHT_WHITE = "\x1B[97m";
    
    // Text styles
    static constexpr std::string BOLD = "\x1B[1m";
    
    static constexpr std::string RESET = "\x1B[0m";
}
