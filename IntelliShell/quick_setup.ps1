# Quick Setup Script for Smart Shell Dependencies
# Run this in PowerShell (as Administrator for best results)

Write-Host "Smart Shell - Quick Setup" -ForegroundColor Green
Write-Host "========================" -ForegroundColor Green
Write-Host ""

# Check if running as admin
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "Note: Some installations may require administrator privileges" -ForegroundColor Yellow
    Write-Host ""
}

# Function to check if command exists
function Test-Command {
    param($Command)
    $null = Get-Command $Command -ErrorAction SilentlyContinue
    return $?
}

# Option 1: Using Chocolatey (easiest)
if (Test-Command choco) {
    Write-Host "✓ Chocolatey found!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Installing dependencies via Chocolatey..." -ForegroundColor Yellow
    
    # Install MinGW-w64
    Write-Host "Installing MinGW-w64..." -ForegroundColor Cyan
    choco install mingw -y
    
    # Install Make
    Write-Host "Installing Make..." -ForegroundColor Cyan
    choco install make -y
    
    # Suggest MSYS2 for ncurses
    Write-Host ""
    Write-Host "For ncurses support, install MSYS2:" -ForegroundColor Yellow
    Write-Host "  choco install msys2 -y" -ForegroundColor White
    Write-Host ""
    Write-Host "Then in MSYS2 terminal:" -ForegroundColor Yellow
    Write-Host "  pacman -S mingw-w64-x86_64-ncurses" -ForegroundColor White
    
} else {
    Write-Host "Chocolatey not found. Install it first:" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Run in PowerShell (as Administrator):" -ForegroundColor Cyan
    Write-Host 'Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString("https://community.chocolatey.org/install.ps1"))' -ForegroundColor White
    Write-Host ""
    Write-Host "Then run this script again." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Alternative: Manual Installation" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "1. Download MSYS2 from: https://www.msys2.org/" -ForegroundColor White
Write-Host "2. Install and open MSYS2 UCRT64 terminal" -ForegroundColor White
Write-Host "3. Run: pacman -Syu" -ForegroundColor White
Write-Host "4. Run: pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-ncurses" -ForegroundColor White
Write-Host "5. Add C:\msys64\ucrt64\bin to PATH" -ForegroundColor White

Write-Host ""
Write-Host "Setup instructions saved to: INSTALL_WINDOWS.md" -ForegroundColor Green
Write-Host ""


