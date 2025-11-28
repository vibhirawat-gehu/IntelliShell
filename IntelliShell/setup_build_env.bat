@echo off
echo ============================================
echo Smart Shell - Build Environment Setup
echo ============================================
echo.

REM Check for Chocolatey
where choco >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Chocolatey not found. Installing Chocolatey...
    echo Please run PowerShell as Administrator and execute:
    echo Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
    echo.
    echo Then run this script again.
    pause
    exit /b 1
)

echo Installing/Updating build tools...
echo.

REM Install/Update MinGW-w64
echo [1/3] Installing MinGW-w64...
choco install mingw -y --version=11.2.0 || choco upgrade mingw -y

REM Install Make
echo [2/3] Installing Make...
choco install make -y

REM Note about ncurses
echo.
echo [3/3] Note about ncurses:
echo ============================================
echo ncurses is not easily available on Windows.
echo Options:
echo 1. Use MSYS2: choco install msys2 -y
echo 2. Build in WSL (Windows Subsystem for Linux)
echo 3. Use PDCurses (Windows-compatible ncurses)
echo ============================================
echo.

REM Check if MSYS2 is installed
where msys2 >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo MSYS2 found! You can install ncurses with:
    echo   msys2 -c "pacman -S mingw-w64-x86_64-ncurses"
) else (
    echo To install MSYS2 (includes ncurses):
    echo   choco install msys2 -y
)

echo.
echo Setup complete!
echo.
echo Next steps:
echo 1. Close and reopen your terminal
echo 2. Verify: g++ --version (should be 8.0+)
echo 3. Verify: make --version
echo 4. For ncurses, use MSYS2 or WSL
echo.
pause


