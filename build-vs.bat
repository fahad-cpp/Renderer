@echo off
setlocal
set "EXENAME=Renderer.exe"
set "PRESET=vs-release"

if not exist build-vs mkdir build-vs
if exist bin\%CONFIG%\%EXENAME% del bin\%CONFIG%\%EXENAME%

where cmake >nul 2>nul
if errorlevel 1 (
    echo "CMake not found."
    exit /b 1
)

cmake --preset %PRESET% 2>build-vs\error.txt
if errorlevel 1 (
    echo Failed to generate build files.
    exit /b 1
)

cmake --build --preset %PRESET% >build-vs\error.txt
if errorlevel 1 (
    echo Failed to build the project.
    exit /b 1
)

bin\%EXENAME%
endlocal
