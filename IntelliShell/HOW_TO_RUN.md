# 🎯 How to Run Smart Shell - Complete Guide

## ✅ You Have Two Options:

### Option A: MSYS2 (Ready to Use!)
### Option B: WSL (Recommended for Linux-like experience)

---

## 🚀 Option A: MSYS2 (Quick Setup)

Since MSYS2 is already installed, follow these steps:

### Step 1: Open MSYS2 UCRT64 Terminal
1. Press **Windows key**
2. Type: `MSYS2 UCRT64`
3. Click on it to open

### Step 2: Install Required Tools
Copy and paste this into MSYS2 terminal:

```bash
pacman -Syu
```

When it asks to close terminal, close and reopen MSYS2 UCRT64, then:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-ncurses
```

Type `Y` when asked to confirm.

### Step 3: Navigate to Your Project
In MSYS2 terminal:

```bash
cd /d/codes/IntelliShell-main
```

### Step 4: Build the Project
```bash
make
```

### Step 5: Run It!
```bash
./smart_shell.exe
```

**Done!** 🎉

---

## 🚀 Option B: WSL (Best Linux Experience)

### Step 1: Install Ubuntu (if needed)
Your WSL currently only has docker-desktop. Install Ubuntu:

```powershell
wsl --install -d Ubuntu
```

### Step 2: Open Ubuntu Terminal
- Type `ubuntu` in PowerShell, OR
- Find "Ubuntu" in Start Menu

### Step 3: Install Build Tools
In Ubuntu terminal:

```bash
sudo apt-get update
sudo apt-get install -y build-essential libncurses5-dev g++ make
```

### Step 4: Go to Project
```bash
cd /mnt/d/codes/IntelliShell-main
```

### Step 5: Build
```bash
make
```

### Step 6: Run
```bash
./smart_shell
```

---

## 📝 Quick Command Summary

### MSYS2 Method:
```bash
# In MSYS2 UCRT64 terminal:
cd /d/codes/IntelliShell-main
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-ncurses
make
./smart_shell.exe
```

### WSL Method:
```bash
# In Ubuntu/WSL terminal:
cd /mnt/d/codes/IntelliShell-main
sudo apt-get update && sudo apt-get install -y build-essential libncurses5-dev g++ make
make
./smart_shell
```

---

## 🎮 Using the Shell

Once running, you'll see:

```
Smart Shell v2.0 - Intelligent Command Interface (C++)
Type 'help' for available commands, 'exit' to quit

user(user)@localhost:~/path$ 
```

**Try these commands:**

```bash
help              # See all commands
ls                # List files  
cd Documents      # Change directory
history           # View command history
suggest ls        # Get command suggestions
resources         # Check system resources (Linux only)
alias ll=ls -la   # Create alias
exit              # Exit shell
```

---

## ⚠️ Why Not Direct Windows Build?

The project needs Unix headers like `sys/wait.h` which aren't in standard Windows MinGW. MSYS2 and WSL provide these.

---

## 🆘 Troubleshooting

### MSYS2 Issues:
- **"command not found"**: Make sure you're in UCRT64 terminal (not MSYS)
- **Build fails**: Run `pacman -Syu` first to update

### WSL Issues:
- **"Ubuntu not found"**: Run `wsl --install -d Ubuntu`
- **"Permission denied"**: Use `sudo` for install commands

---

## ✅ Recommended: Start with MSYS2!

Since MSYS2 is already installed, it's the fastest path:

1. Open **MSYS2 UCRT64**
2. Run: `pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-ncurses`
3. `cd /d/codes/IntelliShell-main`
4. `make`
5. `./smart_shell.exe`

**That's it!** Enjoy your Smart Shell! 🚀




