@echo off
setlocal
set "EXENAME=Renderer.exe"
set "PRESET=clang-release"

if not exist build mkdir build
if exist bin\%CONFIG%\%EXENAME% del bin\%CONFIG%\%EXENAME%

where cmake >nul 2>nul
if errorlevel 1 (
    echo "CMake not found."
    exit /b 1
)

cmake --preset %PRESET%
if errorlevel 1 (
    echo Failed to generate build files.
    exit /b 1
)

cmake --build --preset %PRESET%
if errorlevel 1 (
    echo Failed to build the project.
    exit /b 1
)
if exist "build\%PRESET%\compile_commands.json" copy /Y "build\%PRESET%\compile_commands.json" "build\compile_commands.json" >nul
bin\%CONFIG%\%EXENAME%
endlocal
