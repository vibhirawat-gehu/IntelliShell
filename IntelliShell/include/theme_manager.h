#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <string>
#include <chrono>

enum class ThemeMode {
    IDLE,      // Calm blue animated wave
    HEAVY_CPU, // Glowing red/amber gradient
    NIGHT      // Dark theme with flowing particles
};

class ThemeManager {
private:
    ThemeMode current_mode_;
    double cpu_usage_;
    std::chrono::system_clock::time_point last_update_;
    bool initialized_;
    
    // Theme colors (ANSI codes)
    std::string idle_bg_;
    std::string heavy_cpu_bg_;
    std::string night_bg_;
    std::string idle_prompt_;
    std::string heavy_cpu_prompt_;
    std::string night_prompt_;
    
    void updateThemeBasedOnTime();
    void updateThemeBasedOnCPU();
    
public:
    ThemeManager();
    ~ThemeManager();
    
    bool initialize();
    void update(double cpu_usage);
    ThemeMode getCurrentMode() const { return current_mode_; }
    
    // Visual effects
    void applyTheme();
    void displayIdleWave();
    void displayHeavyCPUGradient();
    void displayNightParticles();
    std::string getPromptColor() const;
    std::string getBackgroundEffect() const;
    void clearBackground();
    
    // Utility
    int getCurrentHour() const;
    bool isNightTime() const;
};

#endif // THEME_MANAGER_H




