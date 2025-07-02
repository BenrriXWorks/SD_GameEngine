# Script de compilación para Windows PowerShell

Write-Host "Compilando sistema de carga de cartas..." -ForegroundColor Green

# Configuración del compilador
$compiler = "g++"
$flags = @(
    "-std=c++23",
    "-Wall", 
    "-Wextra",
    "-O2",
    "-I.",
    "-Ilibs",
    "-DSIMDJSON_IMPLEMENTATION"
)

$sources = @(
    "test_card_loading.cpp",
    "src/cards/CardLoader.cpp",
    "src/game/GameState.cpp",
    "src/effects/EffectDispatch.cpp",
    "src/lex/EffectLexer.cpp"
)

$target = "test_card_loading.exe"

# Compilar
$command = "$compiler $($flags -join ' ') $($sources -join ' ') -o $target"
Write-Host "Ejecutando: $command" -ForegroundColor Yellow

try {
    Invoke-Expression $command
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Compilación exitosa!" -ForegroundColor Green
        Write-Host "Ejecutable generado: $target" -ForegroundColor Cyan
        
        # Ejecutar el test si la compilación fue exitosa
        Write-Host "`nEjecutando test..." -ForegroundColor Yellow
        & ".\$target"
    } else {
        Write-Host "✗ Error en la compilación" -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host "✗ Error: $_" -ForegroundColor Red
    exit 1
}
