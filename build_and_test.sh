#!/bin/bash

# Script de compilación para Linux/WSL

echo -e "\033[32mCompilando sistema de carga de cartas...\033[0m"

# Configuración del compilador
COMPILER="g++"
FLAGS="-std=c++23 -Wall -Wextra -O2 -I. -Ilibs -DSIMDJSON_IMPLEMENTATION"
SOURCES="test_card_loading.cpp src/CardLoader.cpp"
TARGET="test_card_loading"

# Compilar
echo -e "\033[33mEjecutando: $COMPILER $FLAGS $SOURCES -o $TARGET\033[0m"

if $COMPILER $FLAGS $SOURCES -o $TARGET; then
    echo -e "\033[32m✓ Compilación exitosa!\033[0m"
    echo -e "\033[36mEjecutable generado: $TARGET\033[0m"
    
    # Ejecutar el test si la compilación fue exitosa
    echo -e "\n\033[33mEjecutando test...\033[0m"
    ./$TARGET
else
    echo -e "\033[31m✗ Error en la compilación\033[0m"
    exit 1
fi
