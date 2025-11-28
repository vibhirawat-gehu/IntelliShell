# PowerShell script to install dependencies for Smart Shell on Windows
# This script helps set up the development environment

Write-Host "Smart Shell - Windows Dependency Installer" -ForegroundColor Green
Write-Host "==========================================`n" -ForegroundColor Green

# Check for existing tools
Write-Host "Checking existing installations..." -ForegroundColor Yellow

$gcc_version = $null
try {
    $gcc_output = g++ --version 2>&1
    if ($LASTEXITCODE -eq 0) {
        $gcc_version = $gcc_output | Select-String -Pattern '(\d+\.\d+)' | ForEach-Object { $_.Matches[0].Value }
        Write-Host "✓ g++ found: version $gcc_version" -ForegroundColor Green
        
        if ([version]$gcc_version -lt [version]"7.1") {
            Write-Host "  ⚠ Warning: g++ version is older than 7.1, C++17 support may be limited" -ForegroundColor Yellow
        }
    }
} catch {
    Write-Host "✗ g++ not found" -ForegroundColor Red
}

$make_found = $false
try {
    $make_output = make --version 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ make found" -ForegroundColor Green
        $make_found = $true
    }
} catch {
    Write-Host "✗ make not found" -ForegroundColor Red
}

$msys2_path = @(
    "C:\msys64\ucrt64\bin",
    "C:\msys64\mingw64\bin",
    "C:\msys64\usr\bin"
)

$msys2_found = $false
foreach ($path in $msys2_path) {
    if (Test-Path $path) {
        Write-Host "✓ MSYS2 found at: $path" -ForegroundColor Green
        $msys2_found = $true
        break
    }
}

# Check for WSL
$wsl_found = $false
try {
    $wsl_list = wsl --list --quiet 2>&1
    if ($LASTEXITCODE -eq 0 -and $wsl_list.Count -gt 0) {
        Write-Host "✓ WSL found" -ForegroundColor Green
        $wsl_found = $true
    }
} catch {
    Write-Host "✗ WSL not found" -ForegroundColor Yellow
}

Write-Host "`nInstallation Options:" -ForegroundColor Cyan
Write-Host "1. Install/Update via MSYS2 (Recommended for Windows)" -ForegroundColor White
Write-Host "2. Use WSL (If available)" -ForegroundColor White
Write-Host "3. Manual installation guide" -ForegroundColor White
Write-Host "4. Try building with current setup" -ForegroundColor White
Write-Host "`nNote: See INSTALL_WINDOWS.md for detailed instructions" -ForegroundColor Yellow

# Provide next steps
Write-Host "`nRecommended Actions:" -ForegroundColor Cyan
if (-not $gcc_version) {
    Write-Host "  - Install MSYS2 and add to PATH" -ForegroundColor Yellow
    Write-Host "  - Or install MinGW-w64" -ForegroundColor Yellow
}

if (-not $make_found) {
    Write-Host "  - Install make (via MSYS2 or GNUWin32)" -ForegroundColor Yellow
}

Write-Host "`nFor MSYS2 installation:" -ForegroundColor Cyan
Write-Host "  1. Download from: https://www.msys2.org/" -ForegroundColor White
Write-Host "  2. Open MSYS2 UCRT64 terminal" -ForegroundColor White
Write-Host "  3. Run: pacman -Syu" -ForegroundColor White
Write-Host "  4. Run: pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-ncurses" -ForegroundColor White
Write-Host "  5. Add C:\msys64\ucrt64\bin to your PATH" -ForegroundColor White

if ($wsl_found) {
    Write-Host "`nFor WSL installation:" -ForegroundColor Cyan
    Write-Host "  Run in WSL: sudo apt-get install build-essential libncurses5-dev" -ForegroundColor White
}

Write-Host "`nScript completed!" -ForegroundColor Green


