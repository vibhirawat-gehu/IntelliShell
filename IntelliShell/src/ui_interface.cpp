#include "smart_shell.h"

#ifdef _WIN32
// Windows stub implementations
UIManager::UIManager() : main_win_(nullptr), suggestion_win_(nullptr),
                         resource_win_(nullptr), initialized_(false) {
}
UIManager::~UIManager() {
    cleanup();
}
bool UIManager::initialize() { return true; }
void UIManager::cleanup() { initialized_ = false; }
void UIManager::displaySuggestions(const std::vector<Suggestion>&) {}
void UIManager::displayResources(const ResourceInfo&) {}
void UIManager::printMessage(const std::string& msg, int) { std::cout << msg << std::endl; }
void UIManager::printPrompt(const std::string& prompt) { std::cout << prompt; std::cout.flush(); }
void UIManager::printOutput(const std::string& output) { std::cout << output; }
void UIManager::displayHelp() {}
void UIManager::displayResourceSummary(const ResourceInfo&) {}
void UIManager::displayHistory(const std::vector<std::string>&, int) {}
#else
UIManager::UIManager() : main_win_(nullptr), suggestion_win_(nullptr), 
                         resource_win_(nullptr), initialized_(false) {
}

UIManager::~UIManager() {
    cleanup();
}

bool UIManager::initialize() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    
    // Initialize color pairs
    init_pair(1, COLOR_RED, COLOR_BLACK);     // Error messages
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Success messages
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Warnings
    init_pair(4, COLOR_BLUE, COLOR_BLACK);    // Information
    init_pair(5, COLOR_CYAN, COLOR_BLACK);    // Suggestions
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK); // Resources
    
    // Create main window
    main_win_ = newwin(LINES - 4, COLS, 0, 0);
    scrollok(main_win_, TRUE);
    
    // Create suggestion window (bottom)
    suggestion_win_ = newwin(3, COLS, LINES - 3, 0);
    wbkgd(suggestion_win_, COLOR_PAIR(5));
    
    // Create resource window (top right)
    resource_win_ = newwin(8, 40, 0, COLS - 40);
    wbkgd(resource_win_, COLOR_PAIR(6));
    
    initialized_ = true;
    return true;
}

void UIManager::cleanup() {
    if (initialized_) {
        if (main_win_) delwin(main_win_);
        if (suggestion_win_) delwin(suggestion_win_);
        if (resource_win_) delwin(resource_win_);
        endwin();
        initialized_ = false;
    }
}

void UIManager::displaySuggestions(const std::vector<Suggestion>& suggestions) {
    if (!initialized_ || !suggestion_win_) return;
    
    werase(suggestion_win_);
    wbkgd(suggestion_win_, COLOR_PAIR(5));
    
    if (!suggestions.empty()) {
        wprintw(suggestion_win_, "Suggestions: ");
        for (size_t i = 0; i < suggestions.size() && i < 3; i++) {
            wprintw(suggestion_win_, "%s ", suggestions[i].suggestion.c_str());
        }
        if (suggestions.size() > 3) {
            wprintw(suggestion_win_, "...");
        }
    } else {
        wprintw(suggestion_win_, "No suggestions available");
    }
    
    wrefresh(suggestion_win_);
}

void UIManager::displayResources(const ResourceInfo& resources) {
    if (!initialized_ || !resource_win_) return;
    
    werase(resource_win_);
    wbkgd(resource_win_, COLOR_PAIR(6));
    
    wprintw(resource_win_, "System Resources\n");
    wprintw(resource_win_, "================\n");
    wprintw(resource_win_, "CPU: %.1f%%\n", resources.cpu_usage);
    wprintw(resource_win_, "RAM: %ld MB\n", resources.memory_usage / 1024 / 1024);
    wprintw(resource_win_, "Procs: %d\n", resources.process_count);
    
    // Show warnings
    if (resources.cpu_usage > 80.0) {
        wattron(resource_win_, COLOR_PAIR(1));
        wprintw(resource_win_, "HIGH CPU!\n");
        wattroff(resource_win_, COLOR_PAIR(1));
    }
    
    if (resources.memory_usage > 1024 * 1024 * 1024) {
        wattron(resource_win_, COLOR_PAIR(1));
        wprintw(resource_win_, "HIGH RAM!\n");
        wattroff(resource_win_, COLOR_PAIR(1));
    }
    
    wrefresh(resource_win_);
}

void UIManager::printMessage(const std::string& message, int type) {
    if (!initialized_ || !main_win_) {
        // Fallback to regular printf
        switch (type) {
            case 1: std::cout << "\033[31m" << message << "\033[0m" << std::endl; break; // Error
            case 2: std::cout << "\033[32m" << message << "\033[0m" << std::endl; break; // Success
            case 3: std::cout << "\033[33m" << message << "\033[0m" << std::endl; break; // Warning
            case 4: std::cout << "\033[34m" << message << "\033[0m" << std::endl; break; // Info
            default: std::cout << message << std::endl; break;
        }
        return;
    }
    
    wattron(main_win_, COLOR_PAIR(type));
    wprintw(main_win_, "%s\n", message.c_str());
    wattroff(main_win_, COLOR_PAIR(type));
    wrefresh(main_win_);
}

void UIManager::printPrompt(const std::string& prompt) {
    if (!initialized_ || !main_win_) {
        std::cout << prompt;
        std::cout.flush();
        return;
    }
    
    wattron(main_win_, COLOR_PAIR(2));
    wprintw(main_win_, "%s", prompt.c_str());
    wattroff(main_win_, COLOR_PAIR(2));
    wrefresh(main_win_);
}

void UIManager::printOutput(const std::string& output) {
    if (!initialized_ || !main_win_) {
        std::cout << output;
        return;
    }
    
    wprintw(main_win_, "%s", output.c_str());
    wrefresh(main_win_);
}

void UIManager::displayHelp() {
    if (!initialized_ || !main_win_) {
        // Fallback - would need access to ShellState
        return;
    }
    
    werase(main_win_);
    
    wattron(main_win_, COLOR_PAIR(4));
    wprintw(main_win_, "Smart Shell v2.0 - Available Commands:\n\n");
    wattroff(main_win_, COLOR_PAIR(4));
    
    wattron(main_win_, COLOR_PAIR(2));
    wprintw(main_win_, "Built-in Commands:\n");
    wattroff(main_win_, COLOR_PAIR(2));
    wprintw(main_win_, "  cd [directory]     - Change directory\n");
    wprintw(main_win_, "  alias [name=cmd]   - Create or list aliases\n");
    wprintw(main_win_, "  unalias <name>     - Remove alias\n");
    wprintw(main_win_, "  history [count]    - Show command history\n");
    wprintw(main_win_, "  help               - Show this help\n");
    wprintw(main_win_, "  exit               - Exit shell\n");
    wprintw(main_win_, "  resources          - Show system resource usage\n");
    wprintw(main_win_, "  suggest <partial>  - Get command suggestions\n\n");
    
    wprintw(main_win_, "\nPress any key to continue...");
    wrefresh(main_win_);
    getch();
}

void UIManager::displayResourceSummary(const ResourceInfo& resources) {
    if (!initialized_ || !main_win_) {
        return;
    }
    
    werase(main_win_);
    
    wattron(main_win_, COLOR_PAIR(4));
    wprintw(main_win_, "=== System Resource Summary ===\n");
    wattroff(main_win_, COLOR_PAIR(4));
    
    wprintw(main_win_, "CPU Usage: %.1f%%\n", resources.cpu_usage);
    wprintw(main_win_, "Memory Usage: %ld MB\n", resources.memory_usage / 1024 / 1024);
    wprintw(main_win_, "Process Count: %d\n", resources.process_count);
    
    wprintw(main_win_, "\nPress any key to continue...");
    wrefresh(main_win_);
    getch();
}

void UIManager::displayHistory(const std::vector<std::string>& history, int count) {
    if (!initialized_ || !main_win_) {
        return;
    }
    
    werase(main_win_);
    
    wattron(main_win_, COLOR_PAIR(4));
    wprintw(main_win_, "Command History:\n");
    wattroff(main_win_, COLOR_PAIR(4));
    
    int display_count = (count <= 0 || count > static_cast<int>(history.size())) 
                       ? history.size() : count;
    int start = (history.size() > display_count) ? history.size() - display_count : 0;
    
    for (size_t i = start; i < history.size(); i++) {
        wprintw(main_win_, "%4zu  %s\n", i + 1, history[i].c_str());
    }
    
    wprintw(main_win_, "\nPress any key to continue...");
    wrefresh(main_win_);
    getch();
}
#endif // _WIN32


