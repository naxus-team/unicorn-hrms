@echo off
echo Building Unicorn Desktop (OpenGL)...
if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 18 2026" -A x64
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed
    pause
    exit /b 1
)
echo Build complete: build\bin\Release\UnicornDesktop.exe
pause
