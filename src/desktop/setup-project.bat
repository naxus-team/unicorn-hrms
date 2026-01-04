@echo off
echo Building Unicorn Desktop (OpenGL)...
echo.

:: Get script directory and go to project root
set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%\.."

if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 18 2028" -A x64
if errorlevel 1 (
    echo.
    echo ========================================
    echo CMake Configuration Failed!
    echo ========================================
    echo.
    echo Possible reasons:
    echo 1. CMake not installed or not in PATH
    echo 2. Visual Studio 2022 not installed
    echo 3. Missing dependencies (GLFW, GLAD, GLM)
    echo.
    pause
    exit /b 1
)

echo.
echo Building...
echo.

cmake --build . --config Release
if errorlevel 1 (
    echo.
    echo ========================================
    echo Build Failed!
    echo ========================================
    echo Check the errors above
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build Success!
echo ========================================
echo.
echo Executable: build\bin\Release\UnicornDesktop.exe
echo.
echo To run: scripts\run.bat
echo.
pause