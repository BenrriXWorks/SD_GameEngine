# Makefile para compilar el sistema de pruebas unificado

CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O2 -I. -Ilibs -DSIMDJSON_IMPLEMENTATION

# Aplicación principal unificada
TARGET_MAIN = main
SOURCES_MAIN = main.cpp src/api/GameAPI.cpp src/cards/CardLoader.cpp src/game/GameState.cpp src/lex/EffectLexer.cpp

all: $(TARGET_MAIN)

$(TARGET_MAIN): $(SOURCES_MAIN)
	$(CXX) $(CXXFLAGS) $(SOURCES_MAIN) -o $(TARGET_MAIN) 2>&1 | tee compilation_result.temp

clean:
	rm -f $(TARGET_MAIN) compilation_result.temp

test: $(TARGET_MAIN)
	./$(TARGET_MAIN)

.PHONY: all clean test
# Hacer el script ejecutable
make_executable:
	chmod +x build_and_test.sh

.PHONY: all clean test test-api make_executable
