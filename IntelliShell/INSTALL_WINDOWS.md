# Windows Installation Guide for Smart Shell

This guide will help you install all necessary dependencies to build and run Smart Shell on Windows.

## Option 1: Using MSYS2 (Recommended)

MSYS2 provides a Unix-like environment on Windows with a package manager.

### Step 1: Install MSYS2
1. Download MSYS2 from: https://www.msys2.org/
2. Run the installer and follow the instructions
3. After installation, open **MSYS2 UCRT64** terminal (not MSYS2 MSYS)

### Step 2: Update MSYS2
```bash
pacman -Syu
# Close and reopen terminal when prompted, then run:
pacman -Su
```

### Step 3: Install Build Tools
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-make
pacman -S mingw-w64-ucrt-x86_64-ncurses
```

### Step 4: Add to PATH
Add `C:\msys64\ucrt64\bin` to your Windows PATH environment variable.

### Step 5: Build the Project
```bash
cd /d/codes/IntelliShell-main
make
```

---

## Option 2: Using MinGW-w64 Directly

### Step 1: Download MinGW-w64
1. Download from: https://www.mingw-w64.org/downloads/
   - Or use: https://sourceforge.net/projects/mingw-w64/files/

### Step 2: Install MinGW-w64
- Extract to `C:\mingw-w64`
- Add `C:\mingw-w64\bin` to PATH

### Step 3: Install ncurses
1. Download PDCurses (Windows-compatible ncurses):
   - https://github.com/wmcbrine/PDCurses/releases
2. Compile and install, or download pre-built binaries

### Step 4: Install Make
1. Download from: http://gnuwin32.sourceforge.net/packages/make.htm
2. Add to PATH

---

## Option 3: Using WSL (Windows Subsystem for Linux)

If you have WSL installed, you can build natively:

### Step 1: Install WSL (if not installed)
```powershell
wsl --install
```

### Step 2: Install Dependencies in WSL
```bash
sudo apt-get update
sudo apt-get install -y build-essential libncurses5-dev g++ make
```

### Step 3: Build
```bash
cd /mnt/d/codes/IntelliShell-main
make
```

---

## Quick Test Build

Try building with your current setup:

```powershell
# Using build.bat (Windows batch script)
.\build.bat

# Or manually:
g++ -Wall -Wextra -std=c++17 -g -O2 -Iinclude -c src/main.cpp -o obj/main.o
# ... (continue for other files)
```

---

## Important Notes

1. **C++17 Support**: Your current g++ 6.3.0 may not fully support C++17. Consider upgrading to g++ 7.1+ or use MSYS2.

2. **ncurses on Windows**: 
   - The project uses Unix-style ncurses which may not work directly on Windows
   - Consider using PDCurses or building in WSL/MSYS2

3. **Linux-specific Features**: Some features (like `/proc/stat` reading) are Linux-specific and won't work on native Windows. These features will gracefully degrade.

---

## Verification

After installation, verify:
```bash
g++ --version    # Should show version 7.1 or higher
make --version   # Should show GNU Make
```

---

## Troubleshooting

- **"g++: command not found"**: Add MinGW/MSYS2 bin directory to PATH
- **"ncurses.h: No such file"**: Install ncurses development package
- **C++17 features not recognized**: Update to g++ 7.1+ or use `-std=c++14` temporarily


