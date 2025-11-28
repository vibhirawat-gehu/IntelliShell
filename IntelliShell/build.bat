@echo off
echo Building Smart Shell (C++)...

REM Create object directory if it doesn't exist
if not exist obj mkdir obj

REM Compile source files
echo Compiling source files...
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/main.cpp -o obj/main.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/shell_core.cpp -o obj/shell_core.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/builtin_commands.cpp -o obj/builtin_commands.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/suggestion_engine.cpp -o obj/suggestion_engine.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/resource_monitor.cpp -o obj/resource_monitor.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/shortcuts.cpp -o obj/shortcuts.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/ui_interface.cpp -o obj/ui_interface.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/rbac_system.cpp -o obj/rbac_system.o

REM Link the executable
echo Linking executable...
g++ obj/main.o obj/shell_core.o obj/builtin_commands.o obj/suggestion_engine.o obj/resource_monitor.o obj/shortcuts.o obj/ui_interface.o obj/rbac_system.o -o smart_shell.exe -lpthread

if exist smart_shell.exe (
    echo Build successful! Run smart_shell.exe to start the shell.
) else (
    echo Build failed!
    exit /b 1
)

echo.
echo Note: This shell is designed for Linux/Unix systems.
echo Some features may not work properly on Windows.
echo For full functionality, please run on a Linux environment.
