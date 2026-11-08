@echo off
setlocal
cd /d "%~dp0"

rem Run from the Visual Studio x64 Native Tools Command Prompt.
nvcc -O3 -lineinfo -std=c++17 -I ..\..\common host\main_mmult.cu kernels\mmult.cu -o mmult.exe
if errorlevel 1 exit /b 1

endlocal
