@echo off
REM =====================================================
REM  FleetGuardian - Build & Run Script (start.bat)
REM  Single-file version: main.c only
REM  Requires: gcc (MinGW-w64 or similar) in PATH
REM =====================================================

setlocal

set EXE=FleetGuardian.exe

REM Go to the folder where this batch file is located
cd /d "%~dp0"

echo.
echo [1] Compiling FleetGuardian (main.c)...
gcc main.c -o "%EXE%"

if errorlevel 1 (
    echo.
    echo Compilation failed. Check errors above.
    echo Press any key to exit...
    pause >nul
    exit /b 1
)

echo.
echo [2] Build successful. Running %EXE% ...
echo.

"%EXE%"

echo.
echo Program finished. Press any key to close...
pause >nul

endlocal
