#include "smart_shell.h"
#include "theme_manager.h"
#include "autocomplete.h"
#include "dashboard.h"
#include "smart_features.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iterator>
#include <vector>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

// Initialize static members
unsigned long long ResourceMonitor::prev_idle_ = 0;
unsigned long long ResourceMonitor::prev_total_ = 0;

// Global smart corrector
static SmartCorrector g_smart_corrector;

ShellState::ShellState() : running_(false), interactive_(false),
                           theme_manager_(nullptr), autocomplete_engine_(nullptr), dashboard_(nullptr)
{
    resource_monitor_.reset(new ResourceMonitor());
    rbac_system_.reset(new RBACSystem());
    ui_manager_.reset(new UIManager());
    suggestion_engine_.reset(new SuggestionEngine());
    theme_manager_ = new ThemeManager();
    autocomplete_engine_ = new AutocompleteEngine();
    dashboard_ = new CommandDashboard();
}

ShellState::~ShellState()
{
    cleanup();
}

bool ShellState::initialize()
{
    // Get current directory
    char cwd[MAX_COMMAND_LENGTH];
#ifdef _WIN32
    if (_getcwd(cwd, sizeof(cwd)) == nullptr)
    {
#else
    if (getcwd(cwd, sizeof(cwd)) == nullptr)
    {
#endif
        printError("Failed to get current directory");
        return false;
    }
    current_dir_ = cwd;

    // Get home directory
    const char *home = getenv("HOME");
    if (home)
    {
        home_dir_ = home;
    }
    else
    {
        home_dir_ = "/home/user";
    }

    // Initialize resource monitoring
    if (!resource_monitor_->initialize())
    {
        printError("Failed to initialize resource monitoring");
        return false;
    }

    // Initialize RBAC system
    if (!rbac_system_->initialize())
    {
        printError("Failed to initialize RBAC system");
        return false;
    }

    // Load aliases and history for current user
    loadAliases();
    // Load persisted history for current user
    if (rbac_system_)
    {
        std::ifstream hf(makeHistoryFileForUser(rbac_system_->getCurrentUser()));
        std::string line;
        while (std::getline(hf, line) && history_.size() < MAX_HISTORY)
        {
            history_.push_back(line);
        }
    }

    // Set running flag
    running_ = true;

    // Initialize UI if interactive
    if (interactive_ && !ui_manager_->initialize())
    {
        printError("Failed to initialize UI");
        return false;
    }

    // Initialize new features
    if (theme_manager_ && !theme_manager_->initialize())
    {
        printError("Failed to initialize theme manager");
        return false;
    }

    if (dashboard_)
    {
        dashboard_->initialize(resource_monitor_.get(), this);
    }

    // Initialize smart corrector with all available commands
    g_smart_corrector.addCommand("help");
    g_smart_corrector.addCommand("exit");
    g_smart_corrector.addCommand("cd");
    g_smart_corrector.addCommand("alias");
    g_smart_corrector.addCommand("unalias");
    g_smart_corrector.addCommand("history");
    g_smart_corrector.addCommand("resources");
    g_smart_corrector.addCommand("suggest");
    g_smart_corrector.addCommand("users");
    g_smart_corrector.addCommand("roles");
    g_smart_corrector.addCommand("permissions");
    g_smart_corrector.addCommand("login");
    g_smart_corrector.addCommand("logout");
    g_smart_corrector.addCommand("sudo");
    g_smart_corrector.addCommand("useradd");
    g_smart_corrector.addCommand("userdel");
    g_smart_corrector.addCommand("passwd");
    g_smart_corrector.addCommand("ll");
    g_smart_corrector.addCommand("la");
    g_smart_corrector.addCommand("ps");
    g_smart_corrector.addCommand("net");
    g_smart_corrector.addCommand("findf");
    g_smart_corrector.addCommand("clear");
    g_smart_corrector.addCommand("cls");
    g_smart_corrector.addCommand("ls");
    g_smart_corrector.addCommand("dir");
    g_smart_corrector.addCommand("pwd");
    g_smart_corrector.addCommand("cat");
    g_smart_corrector.addCommand("type");
    g_smart_corrector.addCommand("echo");

    return true;
}

void ShellState::cleanup()
{
    // Save aliases
    saveAliases();

    // Save RBAC configuration
    if (rbac_system_)
    {
        rbac_system_->saveConfig();
    }

    // Cleanup UI
    if (interactive_ && ui_manager_)
    {
        ui_manager_->cleanup();
    }

    // Stop resource monitoring
    if (resource_monitor_)
    {
        resource_monitor_->stop();
    }

    // Cleanup new features
    if (dashboard_)
    {
        delete dashboard_;
        dashboard_ = nullptr;
    }
    if (autocomplete_engine_)
    {
        delete autocomplete_engine_;
        autocomplete_engine_ = nullptr;
    }
    if (theme_manager_)
    {
        delete theme_manager_;
        theme_manager_ = nullptr;
    }
}

#ifdef _WIN32
// Helper function to get current prompt string
static std::string getCurrentPromptString(ShellState *shell_state)
{
    if (!shell_state)
        return "$ ";

    std::string user = "guest";
    std::string role = "guest";
    std::string display_dir = shell_state->getCurrentDir();

    // Simplify directory display
    std::string home = shell_state->getHomeDir();
    if (display_dir.find(home) == 0)
    {
        display_dir = "~" + display_dir.substr(home.length());
    }

    // Return simple prompt without colors for redrawing
    return user + "(" + role + ")@" + display_dir + "$ ";
}

static std::string readLineWithTab(AutocompleteEngine *autocomplete_engine, CommandDashboard *dashboard)
{
    std::string input;
    char ch;
    bool in_autocomplete = false;
    std::string partial;
    int selected_index = 0;
    std::vector<CompletionOption> completions;
    int cursor_pos = 0; // Track cursor position in input

    // Enable console input mode for better control
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));

    while (true)
    {
        ch = _getch();

        if (ch == '\r' || ch == '\n')
        {
            // Apply selected completion if in autocomplete mode
            if (in_autocomplete && !completions.empty())
            {
                input = completions[selected_index].text;
                cursor_pos = input.length();
                autocomplete_engine->clearNavigationBar();
                in_autocomplete = false;

                // Redraw the completed input
                std::cout << "\r\033[K";
                std::cout << input;
                std::cout.flush();
            }
            std::cout << std::endl;
            break;
        }
        else if (ch == '\t')
        {
            // Tab completion - VS Code style
            if (!in_autocomplete)
            {
                // First Tab: Show suggestions without applying
                partial = input;
                completions = autocomplete_engine->getCompletions(partial);
                if (!completions.empty())
                {
                    in_autocomplete = true;
                    selected_index = 0;
                    autocomplete_engine->displayNavigationBar(completions, selected_index);
                }
            }
            else
            {
                // Second Tab: Apply the selected completion
                input = completions[selected_index].text;
                cursor_pos = input.length();

                // Redraw input with completion
                std::cout << "\r\033[K";
                std::cout << input;
                std::cout.flush();

                // Clear suggestions and exit autocomplete mode
                autocomplete_engine->clearNavigationBar();
                in_autocomplete = false;
            }
        }
        else if (ch == '\033')
        { // ESC key
            if (in_autocomplete)
            {
                autocomplete_engine->clearNavigationBar();
                in_autocomplete = false;
            }
        }
        else if (ch == '\b' || ch == 127)
        {
            // Backspace
            if (!input.empty() && cursor_pos > 0)
            {
                input.erase(cursor_pos - 1, 1);
                cursor_pos--;

                // Get cursor position to redraw properly
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                GetConsoleScreenBufferInfo(hStdout, &csbi);

                // Move cursor back, print space, move back again
                std::cout << "\b \b";

                // If there are characters after cursor, redraw them
                if (cursor_pos < (int)input.length())
                {
                    std::cout << input.substr(cursor_pos) << " ";
                    // Move cursor back to correct position
                    for (int i = 0; i < (int)input.length() - cursor_pos + 1; i++)
                    {
                        std::cout << "\b";
                    }
                }
                std::cout.flush();

                if (in_autocomplete)
                {
                    autocomplete_engine->clearNavigationBar();
                    in_autocomplete = false;
                }
            }
        }
        else if (ch == 0 || ch == -32 || ch == 224)
        {
            // Special key (arrow keys, function keys)
            // Note: _getch() returns 0 or 224 (0xE0) for extended keys on Windows
            int key = _getch();

            if (key == 60)
            { // F2
                if (dashboard)
                {
                    // Save current input
                    std::string saved_input = input;

                    // Clear navigation bar before toggling dashboard
                    if (in_autocomplete)
                    {
                        autocomplete_engine->clearNavigationBar();
                        in_autocomplete = false;
                    }

                    // Clear current line
                    std::cout << "\r\033[K";
                    std::cout.flush();

                    dashboard->toggle();

                    // Restore input after dashboard closes
                    if (!dashboard->isActive())
                    {
                        std::cout << saved_input;
                        std::cout.flush();
                    }
                }
            }
            else if (in_autocomplete && (key == 75 || key == 72))
            { // Left arrow (75) or Up arrow (72) - previous completion
                selected_index = (selected_index - 1 + completions.size()) % completions.size();
                autocomplete_engine->displayNavigationBar(completions, selected_index);
            }
            else if (in_autocomplete && (key == 77 || key == 80))
            { // Right arrow (77) or Down arrow (80) - next completion
                selected_index = (selected_index + 1) % completions.size();
                autocomplete_engine->displayNavigationBar(completions, selected_index);
            }
            else if (key == 72)
            { // Up arrow - could be used for history
              // Future: implement history navigation
            }
            else if (key == 80)
            { // Down arrow
              // Future: implement history navigation
            }
        }
        else if (ch >= 32 && ch <= 126)
        {
            // Regular printable character
            input.insert(cursor_pos, 1, ch);
            cursor_pos++;

            // Echo the character and any characters after it
            if (cursor_pos < (int)input.length())
            {
                // Insert mode - redraw rest of line
                std::cout << input.substr(cursor_pos - 1);
                // Move cursor back to correct position
                for (int i = 0; i < (int)input.length() - cursor_pos; i++)
                {
                    std::cout << "\b";
                }
            }
            else
            {
                // Append mode - just echo character
                std::cout << ch;
            }
            std::cout.flush();

            if (in_autocomplete)
            {
                autocomplete_engine->clearNavigationBar();
                in_autocomplete = false;
            }
        }
    }

    if (in_autocomplete)
    {
        autocomplete_engine->clearNavigationBar();
    }

    // Restore console mode
    SetConsoleMode(hStdin, mode);

    return input;
}
#else
static std::string readLineWithTab(AutocompleteEngine *autocomplete_engine, CommandDashboard *dashboard)
{
    std::string input;
    char ch;
    // Simplified for non-Windows - just use getline for now
    std::getline(std::cin, input);
    return input;
}
#endif

int ShellState::run()
{
    std::string input;
    Command cmd;

    while (running_)
    {
        // Update theme based on resources
        if (theme_manager_ && resource_monitor_)
        {
            ResourceInfo info = resource_monitor_->getResources();
            theme_manager_->update(info.cpu_usage);
        }

        // Print prompt with theme
        printPrompt();

        // Check for F2 key (before reading input)
        if (interactive_)
        {
#ifdef _WIN32
            if (_kbhit())
            {
                int key = _getch();
                if (key == 0 || key == 224)
                {
                    int ext_key = _getch();
                    if (ext_key == 60)
                    { // F2
                        dashboard_->toggle();
                        continue;
                    }
                }
            }
#endif
        }

        // Read input with tab completion
        if (interactive_ && autocomplete_engine_)
        {
            input = readLineWithTab(autocomplete_engine_, dashboard_);
        }
        else
        {
            if (!std::getline(std::cin, input))
            {
                if (std::cin.eof())
                {
                    std::cout << std::endl;
                    break; // EOF
                }
                printError("Error reading input");
                continue;
            }
        }

        // Skip empty commands
        if (input.empty())
        {
            continue;
        }

        // Add to history and autocomplete
        addToHistory(input);
        if (autocomplete_engine_)
        {
            autocomplete_engine_->addToHistory(input);
        }

        // Expand shortcuts and user aliases before parsing
        std::string processed_input = input;
        if (isShortcutCommand(input.substr(0, input.find(' '))))
        {
            processed_input = expandShortcut(input);
        }
        // Alias expansion
        {
            std::string alias_expanded = expandAlias(processed_input);
            if (!alias_expanded.empty())
            {
                processed_input = alias_expanded;
            }
        }

        // Parse command
        if (!parseCommand(processed_input, cmd))
        {
            printError("Failed to parse command");
            continue;
        }

        // Execute command
        int result = executeCommand(cmd);
        bool shouldSuggest = (result != 0 && result != 1);
        // Also suggest if command looks unknown but returned 0 (Windows quirks)
        if (!shouldSuggest)
        {
            if (!isBuiltinCommand(cmd.command))
            {
                std::vector<std::string> pre_suggestions = g_smart_corrector.suggestCorrections(cmd.command);
                if (!pre_suggestions.empty())
                {
                    shouldSuggest = true;
                }
            }
        }

        if (shouldSuggest)
        { // 1 is normal exit for some commands
            printError("Command execution failed");

            // Suggest corrections for unknown commands
            std::vector<std::string> suggestions = g_smart_corrector.suggestCorrections(cmd.command);
            if (!suggestions.empty())
            {
                std::cout << "\033[33mDid you mean:\033[0m ";
                for (size_t i = 0; i < suggestions.size() && i < 3; i++)
                {
                    std::cout << "\033[36m" << suggestions[i] << "\033[0m";
                    if (i < suggestions.size() - 1 && i < 2)
                    {
                        std::cout << ", ";
                    }
                }
                std::cout << "?" << std::endl;
                // Print a one-line definition for each printed suggestion
                if (suggestion_engine_)
                {
                    size_t maxExplain = std::min<size_t>(3, suggestions.size());
                    for (size_t i = 0; i < maxExplain; ++i)
                    {
                        std::string desc = suggestion_engine_->getCommandDescription(suggestions[i]);
                        if (!desc.empty())
                        {
                            std::cout << "  -> " << suggestions[i] << ": " << desc << std::endl;
                        }
                    }
                }
            }

            // Print direct correction if available (e.g., sl -> ls)
            if (suggestion_engine_)
            {
                std::string correction;
                if (suggestion_engine_->suggestCorrection(cmd.command, correction))
                {
                    std::cout << "Try: \033[36m" << correction << "\033[0m" << std::endl;
                }
                else
                {
                    // If no suggestions and it's a known command (e.g., missing args), explain it
                    std::string desc = suggestion_engine_->getCommandDescription(cmd.command);
                    if (!desc.empty())
                    {
                        std::cout << "  -> " << cmd.command << ": " << desc << std::endl;
                    }
                }
            }
        }
        else if (result == 0)
        {
            // Successful command: increase usage and possibly suggest alias
            command_usage_[cmd.command]++;
            int used = command_usage_[cmd.command];
            if (used == 10 && suggestion_engine_)
            {
                std::string suggested = suggestion_engine_->generateAutoAlias(cmd.command);
                std::string default_alias = suggested.empty() && !cmd.command.empty() ? std::string(1, cmd.command[0]) : suggested;
                // Skip if an alias already exists that maps to this command
                bool already_aliased = false;
                for (const auto &a : aliases_)
                {
                    if (a.command == cmd.command)
                    {
                        already_aliased = true;
                        break;
                    }
                }

                if (!default_alias.empty() && !already_aliased)
                {
                    std::cout << "\033[33mThis command is used often. Create an alias?\033[0m [y/N] ";
                    std::string resp;
                    std::getline(std::cin, resp);
                    if (!resp.empty() && (resp[0] == 'y' || resp[0] == 'Y'))
                    {
                        std::string alias_name = default_alias;
                        if (isBuiltinCommand(alias_name))
                        {
                            alias_name = alias_name + "_"; // avoid conflicts
                        }
                        if (addAlias(alias_name, cmd.command))
                        {
                            saveAliases();
                            std::cout << "Alias created: " << alias_name << " = " << cmd.command << std::endl;
                        }
                    }
                }
            }
        }

        // Update dashboard if active
        if (dashboard_ && dashboard_->isActive())
        {
            dashboard_->update();
        }
    }

    return 0;
}

void ShellState::printPrompt() const
{
    char *hostname = getenv("HOSTNAME");
    if (!hostname)
        hostname = const_cast<char *>("localhost");

    // Get current directory relative to home
    std::string display_dir;
    if (current_dir_.find(home_dir_) == 0)
    {
        display_dir = "~" + current_dir_.substr(home_dir_.length());
    }
    else
    {
        display_dir = current_dir_;
    }

    // Get theme colors
    std::string prompt_color = "\033[32m"; // Default green
    std::string bg_effect = "";

    if (theme_manager_)
    {
        prompt_color = theme_manager_->getPromptColor();
        bg_effect = theme_manager_->getBackgroundEffect();

        // Display subtle theme effects
        switch (theme_manager_->getCurrentMode())
        {
        case ThemeMode::HEAVY_CPU:
            theme_manager_->displayHeavyCPUGradient();
            break;
        case ThemeMode::NIGHT:
            theme_manager_->displayNightParticles();
            break;
        case ThemeMode::IDLE:
            theme_manager_->displayIdleWave();
            break;
        }
    }

    // Check for resource warnings
    if (resource_monitor_ && resource_monitor_->checkWarning())
    {
        std::cout << "\033[31m[WARNING: High resource usage]\033[0m" << std::endl;
    }

    // Show current user and role with theme colors
    std::string user = rbac_system_ ? rbac_system_->getCurrentUser() : "unknown";
    std::string role = rbac_system_ ? rbac_system_->getCurrentRole() : "guest";

    std::cout << prompt_color << user << "\033[0m(\033[33m" << role
              << "\033[0m)@\033[34m" << display_dir << "\033[0m$ ";
    std::cout.flush();
}

bool ShellState::parseCommand(const std::string &input, Command &cmd)
{
    cmd = Command(); // Reset command

    std::string processed_input = input;

    // Check for background execution
    if (!processed_input.empty() && processed_input.back() == '&')
    {
        cmd.background = true;
        processed_input.pop_back();
        processed_input = trimWhitespace(processed_input);
    }

    // Check for input/output redirection
    size_t input_redir = processed_input.find("<");
    size_t output_redir = processed_input.find(">");
    size_t append_redir = processed_input.find(">>");

    if (append_redir != std::string::npos)
    {
        cmd.append_output = true;
        cmd.output_file = trimWhitespace(processed_input.substr(append_redir + 2));
        processed_input = trimWhitespace(processed_input.substr(0, append_redir));
    }
    else if (output_redir != std::string::npos)
    {
        cmd.output_file = trimWhitespace(processed_input.substr(output_redir + 1));
        processed_input = trimWhitespace(processed_input.substr(0, output_redir));
    }

    if (input_redir != std::string::npos)
    {
        cmd.input_file = trimWhitespace(processed_input.substr(input_redir + 1));
        processed_input = trimWhitespace(processed_input.substr(0, input_redir));
    }

    // Parse command and arguments
    std::istringstream iss(processed_input);
    std::vector<std::string> tokens(std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>());

    if (!tokens.empty())
    {
        cmd.command = tokens[0];
        cmd.args.assign(tokens.begin() + 1, tokens.end());
    }

    return true;
}

int ShellState::executeCommand(const Command &cmd)
{
    if (cmd.command.empty())
    {
        return 0;
    }

    // Check RBAC permissions
    if (rbac_system_ && !rbac_system_->checkPermission(cmd.command, rbac_system_->getCurrentUser()))
    {
        printError("Permission denied: Insufficient privileges for this command");
        return -1;
    }

    // Check if it's a built-in command
    if (isBuiltinCommand(cmd.command))
    {
        return executeBuiltin(cmd);
    }
    else
    {
        return executeExternal(cmd);
    }
}

int ShellState::executeBuiltin(const Command &cmd)
{
    if (cmd.command == "cd")
    {
        return cmdCd(cmd);
    }
    else if (cmd.command == "alias")
    {
        return cmdAlias(cmd);
    }
    else if (cmd.command == "unalias")
    {
        return cmdUnalias(cmd);
    }
    else if (cmd.command == "history")
    {
        return cmdHistory(cmd);
    }
    else if (cmd.command == "help")
    {
        return cmdHelp(cmd);
    }
    else if (cmd.command == "exit")
    {
        return cmdExit(cmd);
    }
    else if (cmd.command == "resources")
    {
        return cmdResources(cmd);
    }
    else if (cmd.command == "suggest")
    {
        return cmdSuggest(cmd);
    }
    else if (cmd.command == "users")
    {
        return cmdUsers(cmd);
    }
    else if (cmd.command == "roles")
    {
        return cmdRoles(cmd);
    }
    else if (cmd.command == "permissions")
    {
        return cmdPermissions(cmd);
    }
    else if (cmd.command == "login")
    {
        return cmdLogin(cmd);
    }
    else if (cmd.command == "logout")
    {
        return cmdLogout(cmd);
    }
    else if (cmd.command == "sudo")
    {
        return cmdSudo(cmd);
    }
    else if (cmd.command == "useradd")
    {
        return cmdUserAdd(cmd);
    }
    else if (cmd.command == "userdel")
    {
        return cmdUserDel(cmd);
    }
    else if (cmd.command == "passwd")
    {
        return cmdPasswd(cmd);
    }
    else if (cmd.command == "clearhist")
    {
        return cmdClearHist(cmd);
    }

    return -1;
}

int ShellState::executeExternal(const Command &cmd)
{
#ifdef _WIN32
    // Windows implementation using system()
    std::string full_cmd = cmd.command;
    for (const auto &arg : cmd.args)
    {
        full_cmd += " " + arg;
    }

    if (!cmd.input_file.empty())
    {
        full_cmd = "type " + cmd.input_file + " | " + full_cmd;
    }

    if (!cmd.output_file.empty())
    {
        if (cmd.append_output)
        {
            full_cmd += " >> " + cmd.output_file;
        }
        else
        {
            full_cmd += " > " + cmd.output_file;
        }
    }

    if (cmd.background)
    {
        full_cmd += " &";
    }

    int result = system(full_cmd.c_str());
    return result;
#else
    pid_t pid;
    int status;
    int input_fd = -1, output_fd = -1;

    // Handle input redirection
    if (!cmd.input_file.empty())
    {
        input_fd = open(cmd.input_file.c_str(), O_RDONLY);
        if (input_fd == -1)
        {
            printError("Failed to open input file");
            return -1;
        }
    }

    // Handle output redirection
    if (!cmd.output_file.empty())
    {
        int flags = O_WRONLY | O_CREAT;
        if (cmd.append_output)
        {
            flags |= O_APPEND;
        }
        else
        {
            flags |= O_TRUNC;
        }

        output_fd = open(cmd.output_file.c_str(), flags, 0644);
        if (output_fd == -1)
        {
            printError("Failed to open output file");
            if (input_fd != -1)
                close(input_fd);
            return -1;
        }
    }

    pid = fork();

    if (pid == 0)
    {
        // Child process
        if (input_fd != -1)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (output_fd != -1)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        // Prepare arguments for exec
        std::vector<char *> args;
        args.push_back(const_cast<char *>(cmd.command.c_str()));

        for (const auto &arg : cmd.args)
        {
            args.push_back(const_cast<char *>(arg.c_str()));
        }
        args.push_back(nullptr);

        execvp(cmd.command.c_str(), args.data());
        printError("Failed to execute command");
        exit(1);
    }
    else if (pid > 0)
    {
        // Parent process
        if (input_fd != -1)
            close(input_fd);
        if (output_fd != -1)
            close(output_fd);

        if (!cmd.background)
        {
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }
        else
        {
            std::cout << "[" << pid << "] Background process started" << std::endl;
            return 0;
        }
    }
    else
    {
        printError("Failed to fork process");
        if (input_fd != -1)
            close(input_fd);
        if (output_fd != -1)
            close(output_fd);
        return -1;
    }
#endif
}

bool ShellState::isBuiltinCommand(const std::string &command) const
{
    static const std::vector<std::string> builtins = {
        "cd", "alias", "unalias", "history", "help", "exit", "resources", "suggest",
        "users", "roles", "permissions", "login", "logout", "sudo", "useradd", "userdel", "passwd", "clearhist"};

    return std::find(builtins.begin(), builtins.end(), command) != builtins.end();
}

bool ShellState::isShortcutCommand(const std::string &command) const
{
    static const std::vector<std::string> shortcuts = {
        "ll", "la", "ps", "net", "findf"};

    return std::find(shortcuts.begin(), shortcuts.end(), command) != shortcuts.end();
}

std::string ShellState::expandShortcut(const std::string &input)
{
    size_t space_pos = input.find(' ');
    std::string shortcut = (space_pos != std::string::npos) ? input.substr(0, space_pos) : input;
    std::string args = (space_pos != std::string::npos) ? input.substr(space_pos) : "";

    // Map shortcuts to their expanded commands
    if (shortcut == "ll" || shortcut == "la")
    {
#ifdef _WIN32
        return "dir /a" + args; // Windows equivalent
#else
        return "ls -la" + args;
#endif
    }
    else if (shortcut == "ps")
    {
#ifdef _WIN32
        return "tasklist" + args; // Windows equivalent
#else
        return "ps aux" + args;
#endif
    }
    else if (shortcut == "net")
    {
#ifdef _WIN32
        return "netstat -an" + args; // Windows equivalent
#else
        return "netstat -tuln" + args;
#endif
    }
    else if (shortcut == "findf")
    {
#ifdef _WIN32
        // Windows find is different, use dir /s /b
        if (!args.empty())
        {
            return "dir /s /b *" + trimWhitespace(args) + "*";
        }
        return "dir /s /b" + args;
#else
        return "find . -name" + args;
#endif
    }

    return input; // Return original if no match
}

void ShellState::addToHistory(const std::string &command)
{
#ifdef NO_STD_THREAD
    // No-op
#else
#ifdef NO_STD_THREAD
    // No-op
#else
    std::lock_guard<std::mutex> lock(state_mutex_);
#endif
#endif

    if (history_.size() >= MAX_HISTORY)
    {
        history_.erase(history_.begin());
    }

    history_.push_back(command);

    // Persist to per-user history file
    std::string user = rbac_system_ ? rbac_system_->getCurrentUser() : std::string("anonymous");
    std::ofstream hf(makeHistoryFileForUser(user), std::ios::app);
    if (hf.is_open())
    {
        hf << command << std::endl;
    }
}

std::vector<std::string> ShellState::getHistory(int count) const
{
#ifdef NO_STD_THREAD
    // No-op
#else
#ifdef NO_STD_THREAD
    // No-op
#else
    std::lock_guard<std::mutex> lock(state_mutex_);
#endif
#endif

    if (count <= 0 || count >= static_cast<int>(history_.size()))
    {
        return history_;
    }

    int start = history_.size() - count;
    return std::vector<std::string>(history_.begin() + start, history_.end());
}

bool ShellState::addAlias(const std::string &alias, const std::string &command)
{
#ifdef NO_STD_THREAD
    // No-op
#else
#ifdef NO_STD_THREAD
    // No-op
#else
    std::lock_guard<std::mutex> lock(state_mutex_);
#endif
#endif

    // Check if alias already exists
    auto it = std::find_if(aliases_.begin(), aliases_.end(),
                           [&alias](const Alias &a)
                           { return a.alias_name == alias; });

    if (it != aliases_.end())
    {
        // Update existing alias
        it->command = command;
        return true;
    }

    // Add new alias
    Alias new_alias;
    new_alias.alias_name = alias;
    new_alias.command = command;
    new_alias.usage_count = 0;
    aliases_.push_back(new_alias);
    return true;
}

bool ShellState::removeAlias(const std::string &alias)
{
#ifdef NO_STD_THREAD
    // No-op
#else
#ifdef NO_STD_THREAD
    // No-op
#else
    std::lock_guard<std::mutex> lock(state_mutex_);
#endif
#endif

    auto it = std::find_if(aliases_.begin(), aliases_.end(),
                           [&alias](const Alias &a)
                           { return a.alias_name == alias; });

    if (it != aliases_.end())
    {
        aliases_.erase(it);
        return true;
    }

    return false;
}

std::string ShellState::expandAlias(const std::string &input)
{
#ifdef NO_STD_THREAD
    // No-op
#else
#ifdef NO_STD_THREAD
    // No-op
#else
    std::lock_guard<std::mutex> lock(state_mutex_);
#endif
#endif

    size_t space_pos = input.find(' ');
    size_t alias_len = (space_pos != std::string::npos) ? space_pos : input.length();

    for (auto &alias : aliases_)
    {
        if (alias.alias_name.length() == alias_len &&
            input.substr(0, alias_len) == alias.alias_name)
        {
            alias.usage_count++;

            if (space_pos != std::string::npos)
            {
                return alias.command + input.substr(space_pos);
            }
            else
            {
                return alias.command;
            }
        }
    }

    return "";
}

bool ShellState::loadAliases()
{
    aliases_.clear();
    std::string user = rbac_system_ ? rbac_system_->getCurrentUser() : std::string("anonymous");
    std::ifstream file(makeAliasFileForUser(user));
    if (!file.is_open())
    {
        return true; // No alias file, that's okay
    }

    std::string line;
    while (std::getline(file, line) && aliases_.size() < MAX_HISTORY)
    {
        line = trimWhitespace(line);

        size_t equals = line.find('=');
        if (equals != std::string::npos)
        {
            std::string alias = trimWhitespace(line.substr(0, equals));
            std::string command = trimWhitespace(line.substr(equals + 1));

            if (!alias.empty() && !command.empty())
            {
                addAlias(alias, command);
            }
        }
    }

    return true;
}

bool ShellState::saveAliases() const
{
#ifdef NO_STD_THREAD
    // No-op
#else
#ifdef NO_STD_THREAD
    // No-op
#else
    std::lock_guard<std::mutex> lock(state_mutex_);
#endif
#endif

    std::string user = rbac_system_ ? rbac_system_->getCurrentUser() : std::string("anonymous");
    std::ofstream file(makeAliasFileForUser(user));
    if (!file.is_open())
    {
        printError("Failed to save aliases");
        return false;
    }

    for (const auto &alias : aliases_)
    {
        file << alias.alias_name << "=" << alias.command << std::endl;
    }

    return true;
}

// Utility functions
std::string trimWhitespace(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
    {
        return "";
    }

    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

bool fileExists(const std::string &filename)
{
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(filename.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
#endif
}

bool isDirectory(const std::string &path)
{
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0)
    {
        return S_ISDIR(buffer.st_mode);
    }
    return false;
#endif
}

void printError(const std::string &message)
{
    std::cerr << "\033[31mError: " << message << "\033[0m" << std::endl;
}

void printSuccess(const std::string &message)
{
    std::cout << "\033[32m" << message << "\033[0m" << std::endl;
}
