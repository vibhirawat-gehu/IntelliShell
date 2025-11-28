#!/bin/bash

# Build script for Smart Shell (Linux/Unix) - C++ Version

echo "Building Smart Shell (C++)..."

# Create object directory if it doesn't exist
mkdir -p obj

# Compile source files
echo "Compiling source files..."
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/main.cpp -o obj/main.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/shell_core.cpp -o obj/shell_core.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/builtin_commands.cpp -o obj/builtin_commands.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/suggestion_engine.cpp -o obj/suggestion_engine.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/resource_monitor.cpp -o obj/resource_monitor.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/shortcuts.cpp -o obj/shortcuts.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/ui_interface.cpp -o obj/ui_interface.o
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/rbac_system.cpp -o obj/rbac_system.o

# Link the executable
echo "Linking executable..."
g++ obj/main.o obj/shell_core.o obj/builtin_commands.o obj/suggestion_engine.o obj/resource_monitor.o obj/shortcuts.o obj/ui_interface.o obj/rbac_system.o -o smart_shell -lncurses -lpthread

if [ -f smart_shell ]; then
    echo "Build successful! Run ./smart_shell to start the shell."
    chmod +x smart_shell
else
    echo "Build failed!"
    exit 1
fi

echo "Build completed successfully!"
