@echo off
setlocal
set CONFIG=Release
set ARCH=x64
set GENERATOR="Ninja"
set EXENAME=Renderer.exe

if not exist build mkdir build
if exist build\%EXENAME% del build\%EXENAME%

where cmake >nul 2>nul
if errorlevel 1 (
    echo "CMake not found."
    exit /b 1
)

cmake -G %GENERATOR% -S . -B build -DCMAKE_BUILD_TYPE=%CONFIG% >build\error.txt 2>nul
if errorlevel 1 (
    echo Failed to generate build files.
    exit /b 1
)

cmake --build build --config %CONFIG% --parallel >build\error.txt 2>nul
if errorlevel 1 (
    echo Failed to build the project.
    exit /b 1
)

if exist bin\%CONFIG%\%EXENAME% start bin\%CONFIG%\%EXENAME% 
endlocal