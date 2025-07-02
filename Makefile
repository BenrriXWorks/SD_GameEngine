# Makefile para compilar el test de carga de cartas

CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O2 -I. -Ilibs -DSIMDJSON_IMPLEMENTATION
TARGET = test_card_loading
SOURCES = test_card_loading.cpp src/cards/CardLoader.cpp src/game/GameState.cpp src/effects/EffectDispatch.cpp src/lex/EffectLexer.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

test: $(TARGET)
	./$(TARGET)

# Hacer el script ejecutable
make_executable:
	chmod +x build_and_test.sh

.PHONY: all clean test make_executable
