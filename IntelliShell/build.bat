@echo off
setlocal enabledelayedexpansion

REM IntelliShell - Combined Build & Setup Script
REM This script handles both environment setup and building the project

echo.
echo ╔═══════════════════════════════════════════════════════════════╗
echo ║          IntelliShell - Build ^& Setup Script                ║
echo ╚═══════════════════════════════════════════════════════════════╝
echo.

REM Parse command line arguments
set "MODE=build"
if "%1"=="setup" set "MODE=setup"
if "%1"=="build-only" set "MODE=build-only"
if "%1"=="clean" set "MODE=clean"
if "%1"=="help" goto :help
if "%1"=="--help" goto :help
if "%1"=="-h" goto :help

if "%MODE%"=="build" (
    echo Running in: SETUP + BUILD mode
    echo Checking environment and building project...
    echo.
) else if "%MODE%"=="build-only" (
    echo Running in: BUILD ONLY mode (skipping setup)
    echo.
) else if "%MODE%"=="setup" (
    echo Running in: SETUP ONLY mode
    echo Installing/updating build tools...
    echo.
) else if "%MODE%"=="clean" (
    echo Running in: CLEAN mode
    echo.
    goto :clean_build
)

REM ============================================================================
REM SETUP ENVIRONMENT (if needed)
REM ============================================================================

if "%MODE%"=="build-only" goto :skip_setup

echo [*] Checking build environment...
echo.

REM Check for GCC/G++
where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] g++ not found in PATH
    goto :install_tools
) else (
    for /f "tokens=3" %%A in ('g++ --version ^| findstr /R "[0-9]"') do set "GCC_VERSION=%%A"
    echo [✓] g++ found: !GCC_VERSION!
)

REM Check for Make
where make >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] make not found in PATH
    goto :install_tools
) else (
    echo [✓] make found
)

echo.
goto :skip_setup

:install_tools
echo.
echo ============================================
echo Environment Setup Required
echo ============================================
echo.

REM Check for Chocolatey
where choco >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] Chocolatey not found. 
    echo.
    echo To install Chocolatey, run PowerShell as Administrator and execute:
    echo.
    echo Set-ExecutionPolicy Bypass -Scope Process -Force; ^
[System.Net.ServicePointManager]::SecurityProtocol = ^
[System.Net.ServicePointManager]::SecurityProtocol -bor 3072; ^
iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
    echo.
    echo Then run this script again.
    echo.
    pause
    exit /b 1
)

echo Installing build tools with Chocolatey...
echo.

REM Install/Update MinGW-w64
echo [1/2] Installing MinGW-w64 (this may take a few minutes)...
choco install mingw -y || choco upgrade mingw -y
if %ERRORLEVEL% NEQ 0 (
    echo [!] Failed to install MinGW-w64
    pause
    exit /b 1
)

REM Install Make
echo [2/2] Installing Make...
choco install make -y
if %ERRORLEVEL% NEQ 0 (
    echo [!] Failed to install Make
    pause
    exit /b 1
)

echo.
echo ============================================
echo Additional Setup Information
echo ============================================
echo.
echo ncurses Library:
echo   - ncurses is not easily available on Windows via Chocolatey
echo   - Options:
echo     1. Use MSYS2: choco install msys2 -y
echo     2. Build in WSL (Windows Subsystem for Linux)
echo     3. Use PDCurses (Windows-compatible ncurses alternative)
echo.

REM Check if MSYS2 is installed
where msys2 >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [✓] MSYS2 already installed!
    echo    Install ncurses with: msys2 -c "pacman -S mingw-w64-ucrt-x86_64-ncurses"
) else (
    echo To install MSYS2 (includes ncurses):
    echo   choco install msys2 -y
)

echo.
echo IMPORTANT: Close and reopen your terminal after setup completes!
echo.

if "%MODE%"=="setup" (
    echo Setup completed. Please restart your terminal and run: build.bat
    pause
    exit /b 0
)

echo.
echo ============================================
echo Environment check after setup
echo ============================================
echo.

REM Re-check tools after installation
where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] g++ still not found. Please close and reopen terminal and try again.
    pause
    exit /b 1
)
echo [✓] g++ available

where make >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] make still not found. Please close and reopen terminal and try again.
    pause
    exit /b 1
)
echo [✓] make available

echo.

:skip_setup

REM ============================================================================
REM BUILD PROJECT (if not setup-only)
REM ============================================================================

if "%MODE%"=="setup" (
    echo Setup completed successfully!
    pause
    exit /b 0
)

echo [*] Starting build process...
echo.

REM Create object directory if it doesn't exist
if not exist obj (
    mkdir obj
    echo [✓] Created obj directory
)

echo [*] Compiling source files...
echo.

REM Compile source files with error checking
set "COMPILE_FAILED=0"

setlocal enabledelayedexpansion
for %%F in (
    main
    shell_core
    builtin_commands
    suggestion_engine
    resource_monitor
    shortcuts
    ui_interface
    rbac_system
    dashboard
    smart_features
    autocomplete
    theme_manager
) do (
    if exist src\%%F.cpp (
        echo   Compiling %%F.cpp...
        g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/%%F.cpp -o obj/%%F.o
        if !ERRORLEVEL! NEQ 0 (
            echo [!] Failed to compile %%F.cpp
            set "COMPILE_FAILED=1"
        )
    )
)

if "!COMPILE_FAILED!"=="1" (
    echo.
    echo [!] Compilation failed!
    pause
    exit /b 1
)

echo.
echo [*] Linking executable...

REM Link all object files
g++ obj/main.o obj/shell_core.o obj/builtin_commands.o obj/suggestion_engine.o obj/resource_monitor.o obj/shortcuts.o obj/ui_interface.o obj/rbac_system.o obj/dashboard.o obj/smart_features.o obj/autocomplete.o obj/theme_manager.o -o smart_shell.exe -lpthread

if exist smart_shell.exe (
    echo [✓] Build successful!
    echo.
    echo ╔═══════════════════════════════════════════════════════════════╗
    echo ║                    Build Complete! ✓                         ║
    echo ╚═══════════════════════════════════════════════════════════════╝
    echo.
    echo To run IntelliShell:
    echo   smart_shell.exe
    echo.
    echo For more help, see README.md or run: build.bat help
    echo.
    REM Auto-start if user wants
    echo Would you like to run smart_shell.exe now? (Y/N)
    set /p response=
    if /i "!response!"=="Y" (
        smart_shell.exe
    )
) else (
    echo [!] Build failed!
    echo.
    echo Troubleshooting:
    echo - Check that all source files exist in src/ directory
    echo - Verify g++ is properly installed: g++ --version
    echo - Check obj/ directory has .o files
    echo - Review error messages above
    echo.
    pause
    exit /b 1
)

pause
exit /b 0

REM ============================================================================
REM CLEAN BUILD
REM ============================================================================

:clean_build
echo Cleaning build artifacts...
echo.

if exist obj\ (
    echo Removing obj directory...
    rmdir /s /q obj
    if exist obj\ (
        echo [!] Failed to remove obj directory completely
        echo Please manually delete the obj folder
    ) else (
        echo [✓] Removed obj directory
    )
)

if exist smart_shell.exe (
    echo Removing executable...
    del smart_shell.exe
    if exist smart_shell.exe (
        echo [!] Failed to remove executable
        echo Please manually delete smart_shell.exe
    ) else (
        echo [✓] Removed smart_shell.exe
    )
)

echo.
echo [✓] Clean complete!
echo To rebuild, run: build.bat
echo.
pause
exit /b 0

REM ============================================================================
REM HELP
REM ============================================================================

:help
echo.
echo ╔═══════════════════════════════════════════════════════════════╗
echo ║        IntelliShell - Build Script Help                      ║
echo ╚═══════════════════════════════════════════════════════════════╝
echo.
echo USAGE:
echo   build.bat [COMMAND]
echo.
echo COMMANDS:
echo   (none)          Setup environment (if needed) and build
echo   setup           Setup build environment only
echo   build-only      Build without checking/setting up environment
echo   clean           Remove build artifacts and executable
echo   help, -h, --help   Show this help message
echo.
echo EXAMPLES:
echo   build.bat              - Full build with setup check
echo   build.bat setup        - Install build tools only
echo   build.bat build-only   - Skip setup and build immediately
echo   build.bat clean        - Clean all build files
echo.
echo REQUIREMENTS:
echo   - g++ (C++17 support) - for compilation
echo   - make - for build automation
echo   - (Optional) ncurses - for advanced UI features
echo.
echo WINDOWS SETUP:
echo   For best results, use MSYS2 or WSL:
echo     - MSYS2: choco install msys2 -y
echo     - WSL: wsl --install -d Ubuntu
echo.
echo For more information, see README.md
echo.
pause
exit /b 0

endlocal
