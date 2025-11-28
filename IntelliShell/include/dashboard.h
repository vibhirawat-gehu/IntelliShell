#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <string>
#include <vector>
#include "smart_shell.h"

class CommandDashboard {
private:
    bool is_active_;
    ResourceMonitor* resource_monitor_;
    ShellState* shell_state_;
    
    void drawBorder();
    void drawSystemStats();
    void drawRecentCommands();
    void drawShortcuts();
    void drawSuggestions();
    void clearScreen();
    
public:
    CommandDashboard();
    ~CommandDashboard();
    
    void initialize(ResourceMonitor* monitor, ShellState* shell);
    void show();
    void hide();
    bool isActive() const { return is_active_; }
    
    void toggle();
    void update();
    
    // Dashboard sections
    void render();
    
    // Input handling
    bool handleKey(int key);
};

#endif // DASHBOARD_H




