@echo off
setlocal
set CONFIG=Release
set ARCH=x64
set GENERATOR="Visual Studio 18 2026"
set EXENAME=Renderer.exe

if not exist build-vs mkdir build-vs
if exist bin\%CONFIG%\%EXENAME% del bin\%CONFIG%\%EXENAME%

where cmake >nul 2>nul
if errorlevel 1 (
    echo "CMake not found."
    exit /b 1
)

cmake -G %GENERATOR% -S . -B build-vs -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=%CONFIG% -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 2>build-vs\error.txt
if errorlevel 1 (
    echo Failed to generate build files.
    exit /b 1
)

cmake --build build-vs --config %CONFIG% --parallel >build-vs\error.txt
if errorlevel 1 (
    echo Failed to build the project.
    exit /b 1
)

bin\%CONFIG%\%EXENAME%
endlocal
