@echo off
setlocal
cd /d "%~dp0"

rem Release build for the AWS Tesla T4 GPU.
nvcc -O3 -lineinfo -arch=sm_75 hw2_starter.cu -o "hw2.exe"
if errorlevel 1 exit /b 1

echo Build complete: hw2.exe
endlocal