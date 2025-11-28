# How to Run Smart Shell

## Quick Start Guide

### Step 1: Ensure Build Tools are Available

Open PowerShell in the project directory and run:

```powershell
# Add Chocolatey to PATH (if not already added)
$env:Path += ";C:\ProgramData\chocolatey\bin"

# Verify tools are available
g++ --version
make --version
```

### Step 2: Build the Project

```powershell
# Build the project
make
```

This will:
- Create `obj/` directory with object files
- Compile all C++ source files
- Link them into `smart_shell.exe` (or `smart_shell` on Unix)

### Step 3: Run the Project

```powershell
# Run the shell
.\smart_shell.exe
```

Or simply:
```powershell
.\smart_shell
```

## Detailed Steps

### Building the Project

**Option 1: Using Make (Recommended)**
```powershell
make
```

**Option 2: Using Build Script**
```powershell
.\build.bat
```

**Option 3: Manual Build**
```powershell
mkdir obj
g++ -Wall -Wextra -std=c++14 -g -O2 -Iinclude -c src/main.cpp -o obj/main.o
g++ -Wall -Wextra -std=c++14 -g -O2 -Iinclude -c src/shell_core.cpp -o obj/shell_core.o
g++ -Wall -Wextra -std=c++14 -g -O2 -Iinclude -c src/builtin_commands.cpp -o obj/builtin_commands.o
g++ -Wall -Wextra -std=c++14 -g -O2 -Iinclude -c src/suggestion_engine.cpp -o obj/suggestion_engine.o
g++ -Wall -Wextra -std=c++14 -g -O2 -Iinclude -c src/resource_monitor.cpp -o obj/resource_monitor.o
g++ -Wall -Wextra -std=c++14 -g -O2 -Iinclude -c src/shortcuts.cpp -o obj/shortcuts.o
g++ -Wall -Wextra -std=c++14 -g -O2 -Iinclude -c src/ui_interface.cpp -o obj/ui_interface.o
g++ -Wall -Wextra -std=c++14 -g -O2 -Iinclude -c src/rbac_system.cpp -o obj/rbac_system.o
g++ obj/*.o -o smart_shell.exe -lpthread
```

### Running the Shell

Once built, you'll see a prompt like:
```
Smart Shell v2.0 - Intelligent Command Interface (C++)
Type 'help' for available commands, 'exit' to quit
Features: Smart suggestions, resource monitoring, shortcuts

user(role)@localhost:~$ 
```

### Using the Shell

**Basic Commands:**
```bash
# List directory
ls
ll              # Shortcut for ls -la

# Change directory
cd Documents

# Show help
help

# View history
history 20

# Get suggestions
suggest ls

# Check resources
resources

# Create alias
alias ll=ls -la

# Exit shell
exit
```

**Built-in Commands:**
- `cd [directory]` - Change directory
- `alias [name=cmd]` - Create or list aliases
- `history [count]` - Show command history
- `help` - Show help information
- `exit` - Exit shell
- `resources` - Show system resource usage
- `suggest <partial>` - Get command suggestions
- `users`, `roles`, `permissions` - RBAC management
- `login <username>`, `logout` - User management
- `sudo <command>` - Execute with admin privileges

## Troubleshooting

### "make: command not found"
```powershell
$env:Path += ";C:\ProgramData\chocolatey\bin"
```

### "g++: command not found"
Ensure `D:\MINGW\bin` is in your PATH.

### Build Errors
- Check that all source files are present in `src/` directory
- Verify g++ supports C++14: `g++ --version`
- Try `make clean` then `make` again

### Runtime Errors
- **ncurses errors**: Build will work without ncurses, just UI features disabled
- **Linux-specific features**: Resource monitoring may not work on Windows
- **Permission errors**: Try running in different directory

### Clean Build
```powershell
make clean    # Remove all build artifacts
make          # Build fresh
```

## Example Session

```
PS D:\codes\IntelliShell-main> make
mkdir -p obj
g++ -Wall -Wextra -std=c++14 -g -O2 -Iinclude -c src/main.cpp -o obj/main.o
...
Linking...
Build successful (or with warning about ncurses)

PS D:\codes\IntelliShell-main> .\smart_shell.exe
Smart Shell v2.0 - Intelligent Command Interface (C++)
Type 'help' for available commands, 'exit' to quit

user(user)@localhost:D:\codes\IntelliShell-main$ help
Smart Shell v2.0 - Available Commands:
...

user(user)@localhost:D:\codes\IntelliShell-main$ ls
[directory listing]

user(user)@localhost:D:\codes\IntelliShell-main$ exit
Goodbye!

PS D:\codes\IntelliShell-main>
```

## Next Steps

1. **First Build**: Run `make` to compile
2. **Test Run**: Run `.\smart_shell.exe`
3. **Try Commands**: Type `help` to see available commands
4. **Explore Features**: Try suggestions, aliases, history
5. **Optional**: Install ncurses via MSYS2 for UI features

Enjoy your Smart Shell!


