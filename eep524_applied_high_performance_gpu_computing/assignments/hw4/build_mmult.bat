@echo off
setlocal
cd /d "%~dp0"

rem List of things to modify for easy compile-time iterations
set "TILE_SIZES=8 16 32"

rem Run from the Visual Studio x64 Native Tools Command Prompt.
for %%L in (%TILE_SIZES%) do (
    echo Building with TILE_WIDTH=%%L

    nvcc -O3 -lineinfo -std=c++17 -x cu -D TILE_WIDTH=%%L ^
        -I ^
        ..\..\common ^
        host\main_mmult.cpp ^
        kernels\mmult.cu ^
        -o ^
        mmult_tile%%L.exe

    if errorlevel 1 exit /b 1
)

endlocal
