# 🚀 Quick Start Guide - How to Run Smart Shell

## ⚠️ Important: Windows Compatibility Issue

The project uses Unix/Linux-specific headers (`sys/wait.h`, etc.) that aren't available in standard Windows MinGW. 

**Best Solution**: Use **WSL** (Windows Subsystem for Linux) or **MSYS2** to build and run.

---

## ✅ Option 1: Using WSL (Easiest & Recommended)

### Step 1: Install WSL (if not already installed)
```powershell
wsl --install
```
Restart your computer when prompted.

### Step 2: Open WSL Terminal
- Type `wsl` in PowerShell, or
- Find "Ubuntu" or "WSL" in Start Menu

### Step 3: Navigate to Project
```bash
cd /mnt/d/codes/IntelliShell-main
```

### Step 4: Install Dependencies
```bash
sudo apt-get update
sudo apt-get install -y build-essential libncurses5-dev g++ make
```

### Step 5: Build
```bash
make
```

### Step 6: Run
```bash
./smart_shell
```

**That's it!** 🎉

---

## ✅ Option 2: Using MSYS2 (Good Alternative)

### Step 1: Open MSYS2 UCRT64 Terminal
- Find "MSYS2 UCRT64" in Start Menu (after MSYS2 installation completes)

### Step 2: Update and Install Tools
```bash
pacman -Syu
# Close and reopen terminal if prompted, then:
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-ncurses
```

### Step 3: Navigate to Project
```bash
cd /d/codes/IntelliShell-main
# Or use Windows path:
cd /d/codes/IntelliShell-main
```

### Step 4: Build
```bash
make
```

### Step 5: Run
```bash
./smart_shell.exe
```

---

## 📝 Step-by-Step Commands (WSL - Copy & Paste)

```bash
# 1. Install dependencies
sudo apt-get update && sudo apt-get install -y build-essential libncurses5-dev g++ make

# 2. Go to project directory
cd /mnt/d/codes/IntelliShell-main

# 3. Build
make

# 4. Run
./smart_shell
```

---

## 🎯 First Run Example

Once you run `./smart_shell`, you'll see:

```
Smart Shell v2.0 - Intelligent Command Interface (C++)
Type 'help' for available commands, 'exit' to quit
Features: Smart suggestions, resource monitoring, shortcuts

user(user)@localhost:/mnt/d/codes/IntelliShell-main$ 
```

Try these commands:
```bash
help              # See all commands
ls                # List files
history           # View command history
suggest ls        # Get suggestions
resources         # Check system resources
exit              # Exit shell
```

---

## ❌ Why Not Direct Windows Build?

The current MinGW installation doesn't include:
- `sys/wait.h` (process management)
- Full POSIX support
- Proper Unix system calls

**Solution**: Use WSL or MSYS2 which provide complete Unix-like environment.

---

## 🆘 Quick Troubleshooting

### "WSL not installed"
Run: `wsl --install` in PowerShell as Administrator

### "Permission denied" in WSL
Use `sudo` for installation commands

### "make: command not found"
```bash
sudo apt-get install make
```

### "Build errors"
```bash
sudo apt-get install build-essential
```

---

## 📖 More Information

- `RUN_PROJECT.md` - Detailed running instructions
- `INSTALL_WINDOWS.md` - Complete installation guide
- `README.md` - Project documentation

---

## ✨ Recommended: Use WSL!

It's the easiest way to run this Unix-based project on Windows:

1. `wsl --install`
2. Open WSL terminal
3. `cd /mnt/d/codes/IntelliShell-main`
4. `sudo apt-get install build-essential libncurses5-dev g++ make`
5. `make`
6. `./smart_shell`

**Enjoy your Smart Shell!** 🎉


