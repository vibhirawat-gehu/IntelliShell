# Build Instructions for Windows

## Status of Installed Tools

✅ **g++ (GCC 6.3.0)** - Found at `D:\MINGW\bin\g++.exe`
✅ **Make 4.4.1** - Installed via Chocolatey
⏳ **MSYS2** - Currently installing (for ncurses support)

## Current Status

### ✅ Installed:
- g++ compiler (GCC 6.3.0) - Note: May have limited C++17 support
- Make build tool (4.4.1)

### ⏳ Installing:
- MSYS2 (for ncurses library support)

### ⚠️ Note:
Your g++ version (6.3.0) was released before C++17 was finalized. The Makefile has been configured to use `-std=c++14` which is fully supported by your compiler.

## Building the Project

### Option 1: Using Make (Recommended)
```powershell
# Make sure Chocolatey bin is in PATH
$env:Path += ";C:\ProgramData\chocolatey\bin"

# Build
make
```

### Option 2: Using build.bat
```powershell
.\build.bat
```

### Option 3: Manual Build
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

## Installing ncurses (for UI features)

After MSYS2 installation completes:

1. Open **MSYS2 UCRT64** terminal (not MSYS)
2. Run:
   ```bash
   pacman -Syu
   # Close and reopen if prompted, then:
   pacman -S mingw-w64-ucrt-x86_64-ncurses
   ```

3. Add MSYS2 to your PATH:
   ```powershell
   # Add to system PATH: C:\tools\msys64\ucrt64\bin
   # Or run from MSYS2 terminal
   ```

4. Rebuild:
   ```powershell
   make clean
   make
   ```

## Troubleshooting

### "make: command not found"
- Add Chocolatey bin to PATH:
  ```powershell
  $env:Path += ";C:\ProgramData\chocolatey\bin"
  ```

### "g++: command not found"
- Ensure `D:\MINGW\bin` is in your PATH
- Or use full path: `D:\MINGW\bin\g++.exe`

### C++17 features not working
- The Makefile now uses `-std=c++14` which is compatible with GCC 6.3.0
- To upgrade g++, install MSYS2 and use its g++:
  ```bash
  pacman -S mingw-w64-ucrt-x86_64-gcc
  ```

### ncurses.h: No such file
- Install ncurses via MSYS2 (see above)
- Or build without UI features (shell will work but without ncurses UI)

### Windows-specific limitations
- `/proc/stat` and `/proc/meminfo` are Linux-specific
- Resource monitoring will show 0 or errors on Windows
- Other features should work fine

## Testing the Build

```powershell
# Build
make

# Run
.\smart_shell.exe

# Or if built without .exe extension
.\smart_shell
```

## Next Steps

1. Wait for MSYS2 installation to complete
2. Install ncurses via MSYS2 (optional, for UI features)
3. Build the project: `make`
4. Test: `.\smart_shell.exe`

Enjoy your Smart Shell!


