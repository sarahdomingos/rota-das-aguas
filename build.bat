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

rem A GLFW em external/lib e 32-bit; um g++ 64-bit nao consegue linkar com ela.
g++ -dumpmachine | findstr /i "x86_64" >nul

if %errorlevel% equ 0 (
    echo [ERRO] O g++ encontrado e 64-bit, mas a GLFW em external/lib e 32-bit.
    echo        Coloque um MinGW 32-bit no inicio do PATH. Exemplo:
    echo        set PATH=C:\MinGW\bin;%%PATH%%
    echo.
    pause
    exit /b 1
)

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
    -I"src" ^
    -I"external/include" ^
    !SOURCES! ^
    -L"external/lib" ^
    -lglfw3dll ^
    -lopengl32 ^
    -lglu32 ^
    -lgdi32 ^
    -luser32 ^
    -lshell32 ^
    -static-libgcc ^
    -static-libstdc++ ^
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

rem A glfw3.dll depende da libgcc_s_dw2-1.dll, que fica na pasta do MinGW.
if not exist libgcc_s_dw2-1.dll (
    for /f "delims=" %%G in ('where g++') do (
        if exist "%%~dpGlibgcc_s_dw2-1.dll" (
            echo Copiando libgcc_s_dw2-1.dll...
            copy /Y "%%~dpGlibgcc_s_dw2-1.dll" "libgcc_s_dw2-1.dll" >nul
        )
    )
)

echo Executando jogo...
echo.

.\jogo.exe

echo.
pause