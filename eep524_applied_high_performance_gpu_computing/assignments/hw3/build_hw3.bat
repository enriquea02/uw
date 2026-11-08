@echo off
setlocal
cd /d "%~dp0"

rem Run in the Visual Studio x64 Native Tools Command Prompt with nvcc on PATH.
nvcc -O3 -lineinfo -std=c++17 -I ..\..\common host\main_control_divergence.cu kernels\control_divergence.cu -o control_divergence.exe
if errorlevel 1 exit /b 1

endlocal
