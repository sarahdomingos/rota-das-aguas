@echo off
setlocal enabledelayedexpansion

echo ================================================
echo       Rota das Aguas - Build MinGW 32-bit
echo ================================================
echo.

echo Verificando compilador MinGW...

where g++ >nul 2>&1

if %errorlevel% neq 0 (
    echo [ERRO] g++ nao foi encontrado no PATH.
    echo.
    pause
    exit /b 1
)

echo [OK] g++ encontrado:
g++ --version
echo.

echo Limpando executavel anterior...

if exist jogo.exe (
    del /q jogo.exe
)

echo.
echo Compilando arquivos .cpp...

set "SOURCES="

for /r src %%F in (*.cpp) do (
    set "SOURCES=!SOURCES! "%%F""
)

if "!SOURCES!"=="" (
    echo [ERRO] Nenhum arquivo .cpp foi encontrado em src.
    pause
    exit /b 1
)

g++ -std=c++17 ^
    -I"external/include" ^
    !SOURCES! ^
    -L"external/lib" ^
    -lglfw3dll ^
    -lopengl32 ^
    -lglu32 ^
    -lgdi32 ^
    -luser32 ^
    -lshell32 ^
    -o "jogo.exe"

if %errorlevel% neq 0 (
    echo.
    echo ================================================
    echo [ERRO] Falha na compilacao.
    echo ================================================
    echo.
    pause
    exit /b 1
)

echo.
echo ================================================
echo [SUCESSO] Compilado com sucesso!
echo ================================================
echo.

if not exist glfw3.dll (
    echo Copiando glfw3.dll...
    copy /Y "external\lib\glfw3.dll" "glfw3.dll" >nul
)

echo Executando jogo...
echo.

jogo.exe

echo.
pause