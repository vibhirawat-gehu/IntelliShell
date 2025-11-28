#include "dashboard.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

CommandDashboard::CommandDashboard()
    : is_active_(false), resource_monitor_(nullptr), shell_state_(nullptr)
{
}

CommandDashboard::~CommandDashboard()
{
    hide();
}

void CommandDashboard::initialize(ResourceMonitor *monitor, ShellState *shell)
{
    resource_monitor_ = monitor;
    shell_state_ = shell;
}

void CommandDashboard::show()
{
    is_active_ = true;
    render();
}

void CommandDashboard::hide()
{
    if (is_active_)
    {
        is_active_ = false;
        clearScreen();
        // Restore the prompt after hiding dashboard
        if (shell_state_)
        {
            shell_state_->printPrompt();
        }
    }
}

void CommandDashboard::toggle()
{
    if (is_active_)
    {
        hide();
    }
    else
    {
        show();
    }
}

void CommandDashboard::render()
{
    if (!is_active_)
        return;

    clearScreen();

    // Use simple ASCII characters for maximum compatibility
    std::cout << "\033[1;1H";          // Move to top
    std::cout << "\033[44m\033[1;37m"; // Blue background, white text
    std::cout << "+============================================================================+\n";
    std::cout << "|                    SMART SHELL DASHBOARD [F2 to close]                    |\n";
    std::cout << "+============================================================================+\033[0m\n";

    // Current session info
    if (shell_state_ && shell_state_->getRBACSystem())
    {
        std::string u = shell_state_->getRBACSystem()->getCurrentUser();
        std::string r = shell_state_->getRBACSystem()->getCurrentRole();
        std::cout << "\n\033[1;36m>> Session:\033[0m\n";
        std::cout << "  User: \033[33m" << u << "\033[0m    Role: \033[33m" << r << "\033[0m\n";
    }

    drawSystemStats();
    drawRecentCommands();
    drawShortcuts();
    // Show user aliases
    if (shell_state_)
    {
        auto aliases = shell_state_->getAliases();
        std::cout << "\n\033[1;36m>> Aliases (current user):\033[0m\n";
        if (aliases.empty())
        {
            std::cout << "  \033[90m(No aliases)\033[0m\n";
        }
        else
        {
            int shown = 0;
            for (const auto &a : aliases)
            {
                std::cout << "  \033[36m" << a.alias_name << "\033[0m = \033[90m" << a.command << "\033[0m";
                std::cout << "  (used " << a.usage_count << ")\n";
                if (++shown >= 10) break;
            }
        }
    }
    drawSuggestions();

    std::cout << "\033[44m\033[1;37m"; // Blue background, white text
    std::cout << "+============================================================================+\033[0m\n";

    std::cout << "\n\033[1;33mPress F2 to close dashboard and return to shell...\033[0m\n";
    std::cout.flush();
}

void CommandDashboard::drawBorder()
{
    // Border is drawn in render()
}

void CommandDashboard::drawSystemStats()
{
    if (!resource_monitor_)
        return;

    std::cout << "\n\033[1;36m>> System Resources:\033[0m\n";

    ResourceInfo info = resource_monitor_->getResources();

    // CPU Usage
    std::cout << "  CPU Usage: " << std::fixed << std::setprecision(1)
              << info.cpu_usage << "%  ";

    int cpu_bar_width = 40;
    int cpu_filled = static_cast<int>(info.cpu_usage / 100.0 * cpu_bar_width);
    std::cout << "[";
    for (int i = 0; i < cpu_bar_width; i++)
    {
        if (i < cpu_filled)
        {
            std::cout << (info.cpu_usage > 70 ? "\033[31m#" : "\033[32m#");
        }
        else
        {
            std::cout << "\033[90m-";
        }
    }
    std::cout << "\033[0m]\n";

    // Memory Usage (already in MB from resource monitor)
    std::cout << "  Memory:    " << std::setw(8) << std::fixed << std::setprecision(1)
              << info.memory_usage << " MB  ";

    int mem_bar_width = 40;
    // Assume 16GB max for visualization (16384 MB)
    int mem_filled = std::min(mem_bar_width, static_cast<int>((info.memory_usage * mem_bar_width) / 16384));
    std::cout << "[";
    for (int i = 0; i < mem_bar_width; i++)
    {
        if (i < mem_filled)
        {
            std::cout << "\033[33m#";
        }
        else
        {
            std::cout << "\033[90m-";
        }
    }
    std::cout << "\033[0m]\n";

    std::cout << "  Processes: " << info.process_count << "\n";
}

void CommandDashboard::drawRecentCommands()
{
    if (!shell_state_)
        return;

    std::cout << "\n\033[1;36m>> Recent Commands:\033[0m\n";

    auto history = shell_state_->getHistory(5);
    int count = 0;
    for (auto it = history.rbegin(); it != history.rend() && count < 5; ++it, ++count)
    {
        std::cout << "  " << std::setw(2) << (count + 1) << ". \033[90m"
                  << (*it).substr(0, 70) << "\033[0m\n";
    }

    if (count == 0)
    {
        std::cout << "  \033[90m(No commands in history)\033[0m\n";
    }
}

void CommandDashboard::drawShortcuts()
{
    std::cout << "\n\033[1;36m>> Quick Shortcuts:\033[0m\n";

    std::vector<std::pair<std::string, std::string>> shortcuts = {
        {"ll/la", "Detailed file listing (dir /a on Windows)"},
        {"ps", "Process list (tasklist on Windows)"},
        {"net", "Network connections (netstat -an)"},
        {"findf <pattern>", "Find files by name"}};

    for (const auto &sc : shortcuts)
    {
        std::cout << "  \033[36m" << std::left << std::setw(18) << sc.first
                  << "\033[0m => \033[90m" << sc.second << "\033[0m\n";
    }
}

void CommandDashboard::drawSuggestions()
{
    if (!shell_state_ || !shell_state_->getSuggestionEngine())
        return;

    std::cout << "\n\033[1;36m>> Quick Commands:\033[0m\n";

    std::vector<std::string> common = {"help", "history", "resources", "suggest", "alias", "users"};
    std::cout << "  ";
    for (size_t i = 0; i < common.size(); i++)
    {
        std::cout << "\033[33m" << common[i] << "\033[0m";
        if (i < common.size() - 1)
            std::cout << " | ";
    }
    std::cout << "\n";
}

void CommandDashboard::clearScreen()
{
    // Clear screen and move to top
    std::cout << "\033[2J\033[H";
    std::cout.flush();
}

void CommandDashboard::update()
{
    if (is_active_)
    {
        render();
    }
}

bool CommandDashboard::handleKey(int key)
{
    if (key == 0x3C00 || key == 'F' + 2)
    { // F2 key
        toggle();
        return true;
    }
    return false;
}

