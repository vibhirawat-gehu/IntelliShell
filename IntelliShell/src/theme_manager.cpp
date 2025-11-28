#include "theme_manager.h"
#include <ctime>
#include <iostream>
#include <iomanip>
#include <cmath>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

ThemeManager::ThemeManager() : current_mode_(ThemeMode::IDLE), cpu_usage_(0.0), initialized_(false) {
    last_update_ = std::chrono::system_clock::now();
    
    // Initialize ANSI color codes for themes
    idle_bg_ = "\033[48;2;30;60;100m";      // Calm blue
    heavy_cpu_bg_ = "\033[48;2;200;50;30m"; // Red/amber
    night_bg_ = "\033[48;2;10;10;20m";       // Dark blue
    
    idle_prompt_ = "\033[94m";      // Bright blue
    heavy_cpu_prompt_ = "\033[91m"; // Bright red
    night_prompt_ = "\033[95m";     // Magenta (dark theme)
}

ThemeManager::~ThemeManager() {
    clearBackground();
}

bool ThemeManager::initialize() {
    initialized_ = true;
    update(0.0);
    return true;
}

void ThemeManager::update(double cpu_usage) {
    cpu_usage_ = cpu_usage;
    last_update_ = std::chrono::system_clock::now();
    
    updateThemeBasedOnTime();
    updateThemeBasedOnCPU();
    applyTheme();
}

void ThemeManager::updateThemeBasedOnTime() {
    int hour = getCurrentHour();
    
    // Night time: 10 PM to 6 AM
    if (hour >= 22 || hour < 6) {
        if (current_mode_ != ThemeMode::HEAVY_CPU) {
            current_mode_ = ThemeMode::NIGHT;
        }
    }
}

void ThemeManager::updateThemeBasedOnCPU() {
    // Heavy CPU: > 70%
    if (cpu_usage_ > 70.0) {
        current_mode_ = ThemeMode::HEAVY_CPU;
    } 
    // If not night and not heavy CPU, use idle mode
    else if (current_mode_ == ThemeMode::NIGHT && !isNightTime()) {
        current_mode_ = ThemeMode::IDLE;
    } else if (current_mode_ != ThemeMode::NIGHT) {
        current_mode_ = ThemeMode::IDLE;
    }
}

void ThemeManager::applyTheme() {
    if (!initialized_) return;
    
    // Apply subtle background effect (for terminals that support it)
    std::string bg_effect = getBackgroundEffect();
    
    // Update prompt colors are handled in printPrompt
}

std::string ThemeManager::getPromptColor() const {
    switch (current_mode_) {
        case ThemeMode::HEAVY_CPU:
            return "\033[91m"; // Bright red
        case ThemeMode::NIGHT:
            return "\033[95m"; // Magenta
        case ThemeMode::IDLE:
        default:
            return "\033[94m"; // Bright blue
    }
}

std::string ThemeManager::getBackgroundEffect() const {
    // ASCII art background effects
    switch (current_mode_) {
        case ThemeMode::HEAVY_CPU:
            return "\033[31m●\033[33m●\033[0m "; // Red/amber particles
        case ThemeMode::NIGHT:
            return "\033[35m·\033[0m "; // Purple particles
        case ThemeMode::IDLE:
        default:
            return "\033[36m~\033[0m "; // Blue wave
    }
}

void ThemeManager::displayIdleWave() {
    if (current_mode_ != ThemeMode::IDLE) return;
    
    static int wave_pos = 0;
    wave_pos = (wave_pos + 1) % 20;
    
    // Subtle wave animation indicator (shown in status line)
    std::string wave = getBackgroundEffect();
}

void ThemeManager::displayHeavyCPUGradient() {
    if (current_mode_ != ThemeMode::HEAVY_CPU) return;
    
    // Show warning indicator
    std::cout << "\033[31m[CPU: " << std::fixed << std::setprecision(1) 
              << cpu_usage_ << "%]\033[0m ";
}

void ThemeManager::displayNightParticles() {
    if (current_mode_ != ThemeMode::NIGHT) return;
    
    // Subtle particle effect indicator
    std::string particle = getBackgroundEffect();
}

void ThemeManager::clearBackground() {
    std::cout << "\033[0m"; // Reset all attributes
}

int ThemeManager::getCurrentHour() const {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&time);
    return local_time ? local_time->tm_hour : 12;
}

bool ThemeManager::isNightTime() const {
    int hour = getCurrentHour();
    return (hour >= 22 || hour < 6);
}




