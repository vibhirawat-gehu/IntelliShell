#include "smart_shell.h"
#include <ctime>
#include <sstream>
#include <iomanip>

// Built-in command implementations
int ShellState::cmdCd(const Command &cmd)
{
    std::string target_dir;

    if (cmd.args.empty())
    {
        target_dir = home_dir_;
    }
    else
    {
        target_dir = cmd.args[0];
    }

#ifdef _WIN32
    if (_chdir(target_dir.c_str()) == 0)
    {
        char cwd[MAX_COMMAND_LENGTH];
        if (_getcwd(cwd, sizeof(cwd)) != nullptr)
        {
            current_dir_ = cwd;
            return 0;
        }
        else
        {
            printError("Failed to get current directory");
            return -1;
        }
    }
    else
    {
        printError("No such directory");
        return -1;
    }
#else
    if (chdir(target_dir.c_str()) == 0)
    {
        char cwd[MAX_COMMAND_LENGTH];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
        {
            current_dir_ = cwd;
            return 0;
        }
        else
        {
            printError("Failed to get current directory");
            return -1;
        }
    }
    else
    {
        printError("No such directory");
        return -1;
    }
#endif
}

int ShellState::cmdAlias(const Command &cmd)
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(state_mutex_);
#endif

    if (cmd.args.empty())
    {
        // List all aliases
        std::cout << "Current aliases:" << std::endl;
        for (const auto &alias : aliases_)
        {
            std::cout << "  " << alias.alias_name << " = " << alias.command
                      << " (used " << alias.usage_count << " times)" << std::endl;
        }
        return 0;
    }
    else if (cmd.args.size() == 1)
    {
        // Show specific alias
        for (const auto &alias : aliases_)
        {
            if (alias.alias_name == cmd.args[0])
            {
                std::cout << alias.alias_name << " = " << alias.command << std::endl;
                return 0;
            }
        }
        std::cout << "Alias '" << cmd.args[0] << "' not found" << std::endl;
        return -1;
    }
    else
    {
        // Create new alias
        std::string alias_cmd;
        for (size_t i = 1; i < cmd.args.size(); i++)
        {
            if (i > 1)
                alias_cmd += " ";
            alias_cmd += cmd.args[i];
        }

        // Prevent overriding built-in commands with aliases
        if (isBuiltinCommand(cmd.args[0]))
        {
            printError("Cannot alias built-in command");
            return -1;
        }

        if (addAlias(cmd.args[0], alias_cmd))
        {
            std::cout << "Alias created: " << cmd.args[0] << " = " << alias_cmd << std::endl;
            return 0;
        }
        else
        {
            printError("Failed to create alias");
            return -1;
        }
    }
}

int ShellState::cmdUnalias(const Command &cmd)
{
    if (cmd.args.empty())
    {
        printError("Usage: unalias <alias_name>");
        return -1;
    }

    if (removeAlias(cmd.args[0]))
    {
        std::cout << "Alias '" << cmd.args[0] << "' removed" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "Alias '" << cmd.args[0] << "' not found" << std::endl;
        return -1;
    }
}

int ShellState::cmdHistory(const Command &cmd)
{
    int count = -1;
    std::string target_user;

    // Parse args: history [count] [--user USERNAME]
    for (size_t i = 0; i < cmd.args.size(); ++i)
    {
        if (cmd.args[i] == "--user" && i + 1 < cmd.args.size())
        {
            target_user = cmd.args[i + 1];
            ++i;
        }
        else if (count == -1)
        {
            try
            {
                count = std::stoi(cmd.args[i]);
            }
            catch (...)
            {
                // ignore non-integer
            }
        }
    }

    // Admin can view other users' histories
    if (!target_user.empty())
    {
        if (!rbac_system_ || rbac_system_->getCurrentRole() != "admin")
        {
            printError("Permission denied: Admin required to view other users' history");
            return -1;
        }

        std::ifstream f(makeHistoryFileForUser(target_user));
        if (!f.is_open())
        {
            printError("No history found for user");
            return -1;
        }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(f, line))
        {
            lines.push_back(line);
        }
        if (count > 0 && count < (int)lines.size())
        {
            lines = std::vector<std::string>(lines.end() - count, lines.end());
        }
        std::cout << "Command history for " << target_user << ":" << std::endl;
        int start_num = (int)lines.size() > 0 ? (int)lines.size() - (int)lines.size() + 1 : 1;
        for (size_t i = 0; i < lines.size(); ++i)
        {
            std::cout << std::setw(4) << (start_num + (int)i) << "  " << lines[i] << std::endl;
        }
        return 0;
    }

    if (count <= 0)
        count = -1;

    auto history = getHistory(count);

    std::cout << "Command history:" << std::endl;
    int start_num = (history_.size() > history.size()) ? history_.size() - (int)history.size() + 1 : 1;
    for (size_t i = 0; i < history.size(); i++)
    {
        std::cout << std::setw(4) << (start_num + (int)i) << "  " << history[i] << std::endl;
    }

    return 0;
}

int ShellState::cmdHelp(const Command &cmd)
{
    std::cout << "Smart Shell v2.0 - Available Commands:\n"
              << std::endl;
    std::cout << "Built-in Commands:" << std::endl;
    std::cout << "  cd [directory]     - Change directory" << std::endl;
    std::cout << "  alias [name=cmd]   - Create or list aliases" << std::endl;
    std::cout << "  unalias <name>     - Remove alias" << std::endl;
    std::cout << "  history [count]    - Show command history" << std::endl;
    std::cout << "  help               - Show this help" << std::endl;
    std::cout << "  exit               - Exit shell" << std::endl;
    std::cout << "  resources          - Show system resource usage" << std::endl;
    std::cout << "  suggest <partial>  - Get command suggestions" << std::endl;
    std::cout << "  users              - List system users (admin only)" << std::endl;
    std::cout << "  roles              - List system roles" << std::endl;
    std::cout << "  permissions        - List available permissions" << std::endl;
    std::cout << "  login <username> <password> - Login with credentials" << std::endl;
    std::cout << "  logout             - Logout current user" << std::endl;
    std::cout << "  sudo <command>     - Execute command with admin privileges" << std::endl;
    std::cout << "  useradd <u> <p> <role> - Create user (admin only)" << std::endl;
    std::cout << "  userdel <u>        - Delete user (admin only)" << std::endl;
    std::cout << "  passwd <u> <p>     - Set password (admin or self)" << std::endl;
    std::cout << "  clearhist          - Delete per-user history files (admin only)\n" << std::endl;

    std::cout << "Default Users:" << std::endl;
    std::cout << "  admin              - Full system access (password: admin@123)" << std::endl;
    std::cout << "  user               - Regular user access (password: user)" << std::endl;
    std::cout << "  guest              - Limited access (password: guest)\n"
              << std::endl;

    std::cout << "Shortcuts:" << std::endl;
    std::cout << "  ll                 - ls -la (detailed listing)" << std::endl;
    std::cout << "  la                 - ls -la (detailed listing)" << std::endl;
    std::cout << "  ps                 - ps aux (process list)" << std::endl;
    std::cout << "  net                - netstat -tuln (network status)" << std::endl;
    std::cout << "  findf <pattern>    - find . -name <pattern>\n"
              << std::endl;

    std::cout << "Features:" << std::endl;
    std::cout << "  - Smart command suggestions" << std::endl;
    std::cout << "  - Resource monitoring with warnings" << std::endl;
    std::cout << "  - Auto-aliasing for frequently used commands" << std::endl;
    std::cout << "  - Command history with search" << std::endl;
    std::cout << "  - Background process execution (&)" << std::endl;
    std::cout << "  - Input/output redirection (< > >>)" << std::endl;
    std::cout << "  - Tab completion for commands" << std::endl;
    std::cout << "  - F2 key to toggle dashboard\n"
              << std::endl;

    std::cout << "Keyboard Shortcuts:" << std::endl;
    std::cout << "  Tab                - Cycle through command completions" << std::endl;
    std::cout << "  F2                 - Toggle system dashboard" << std::endl;
    std::cout << "  Backspace          - Delete character" << std::endl;
    std::cout << "  ESC                - Cancel tab completion" << std::endl;

    return 0;
}

int ShellState::cmdExit(const Command &cmd)
{
    (void)cmd; // Unused
    running_ = false;
    std::cout << "Goodbye!" << std::endl;
    return 1; // Special return code to indicate exit
}

int ShellState::cmdResources(const Command &cmd)
{
    (void)cmd; // Unused

    if (resource_monitor_)
    {
        resource_monitor_->updateResources();
        resource_monitor_->printSummary();
    }
    else
    {
        printError("Resource monitor not initialized");
        return -1;
    }

    return 0;
}

int ShellState::cmdSuggest(const Command &cmd)
{
    if (cmd.args.empty())
    {
        printError("Usage: suggest <partial_command>");
        return -1;
    }

    if (!suggestion_engine_)
    {
        printError("Suggestion engine not initialized");
        return -1;
    }

    auto suggestions = suggestion_engine_->getSuggestions(cmd.args[0]);

    if (!suggestions.empty())
    {
        std::cout << "Suggestions for '" << cmd.args[0] << "':" << std::endl;
        for (const auto &sug : suggestions)
        {
            std::cout << "  " << sug.suggestion << " - " << sug.description
                      << " (confidence: " << sug.confidence << "%)" << std::endl;
        }
        return 0;
    }
    else
    {
        // Try typo correction and show suggestion
        std::string correction;
        if (suggestion_engine_->suggestCorrection(cmd.args[0], correction))
        {
            std::string desc = suggestion_engine_->getCommandDescription(correction);
            if (!desc.empty())
            {
                std::cout << "Did you mean: \033[36m" << correction << "\033[0m\n  -> " << correction << ": " << desc << std::endl;
                return 0;
            }
        }
        std::cout << "No suggestions found for '" << cmd.args[0] << "'" << std::endl;
        return -1;
    }
}

int ShellState::cmdUsers(const Command &cmd)
{
    (void)cmd; // Unused

    if (rbac_system_)
    {
        rbac_system_->listUsers();
        return 0;
    }
    else
    {
        printError("RBAC system not initialized");
        return -1;
    }
}

int ShellState::cmdRoles(const Command &cmd)
{
    (void)cmd; // Unused

    if (rbac_system_)
    {
        rbac_system_->listRoles();
        return 0;
    }
    else
    {
        printError("RBAC system not initialized");
        return -1;
    }
}

int ShellState::cmdPermissions(const Command &cmd)
{
    (void)cmd; // Unused

    if (rbac_system_)
    {
        rbac_system_->listPermissions();
        return 0;
    }
    else
    {
        printError("RBAC system not initialized");
        return -1;
    }
}

int ShellState::cmdLogin(const Command &cmd)
{
    if (cmd.args.size() < 2)
    {
        printError("Usage: login <username> <password>");
        return -1;
    }

    if (rbac_system_)
    {
        // Save current user's aliases before switching user
        saveAliases();

        if (rbac_system_->loginUser(cmd.args[0], cmd.args[1]))
        {
            // Reload per-user aliases and history
            aliases_.clear();
            loadAliases();
            // Clear in-memory history and load new user's history file
            history_.clear();
            std::ifstream hf(makeHistoryFileForUser(rbac_system_->getCurrentUser()));
            std::string line;
            while (std::getline(hf, line) && history_.size() < MAX_HISTORY)
            {
                history_.push_back(line);
            }
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        printError("RBAC system not initialized");
        return -1;
    }
}

int ShellState::cmdLogout(const Command &cmd)
{
    (void)cmd; // Unused

    if (rbac_system_)
    {
        // Save current user's aliases before logging out
        saveAliases();

        if (rbac_system_->logoutUser())
        {
            // Switch to guest context, reload guest aliases/history
            aliases_.clear();
            loadAliases();
            history_.clear();
            std::ifstream hf(makeHistoryFileForUser("anonymous"));
            std::string line;
            while (std::getline(hf, line) && history_.size() < MAX_HISTORY)
            {
                history_.push_back(line);
            }
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        printError("RBAC system not initialized");
        return -1;
    }
}

int ShellState::cmdSudo(const Command &cmd)
{
    if (cmd.args.empty())
    {
        printError("Usage: sudo <command>");
        return -1;
    }

    if (!rbac_system_)
    {
        printError("RBAC system not initialized");
        return -1;
    }

    // Check if current user has admin privileges
    if (rbac_system_->getCurrentRole() != "admin")
    {
        printError("Permission denied: Admin privileges required");
        return -1;
    }

    // Execute command with admin privileges
    Command admin_cmd;
    admin_cmd.command = cmd.args[0];
    admin_cmd.args.assign(cmd.args.begin() + 1, cmd.args.end());

    std::cout << "Executing with admin privileges: " << cmd.args[0] << std::endl;
    return executeExternal(admin_cmd);
}

int ShellState::cmdClearHist(const Command &cmd)
{
    (void)cmd;
    if (!rbac_system_ || rbac_system_->getCurrentRole() != "admin")
    {
        printError("Permission denied: Admin required");
        return -1;
    }

    // Delete common per-user history files we create
    std::vector<std::string> candidates = {
        makeHistoryFileForUser("anonymous"),
        makeHistoryFileForUser("admin"),
        makeHistoryFileForUser("user"),
        makeHistoryFileForUser("guest")
    };

    // Also try current user
    candidates.push_back(makeHistoryFileForUser(rbac_system_->getCurrentUser()));

    int removed = 0;
    for (const auto &f : candidates)
    {
        if (fileExists(f))
        {
            if (std::remove(f.c_str()) == 0)
            {
                removed++;
            }
        }
    }

    // Clear in-memory history
    {
        history_.clear();
    }

    std::cout << "Cleared " << removed << " history file(s)" << std::endl;
    return 0;
}

int ShellState::cmdUserAdd(const Command &cmd)
{
    if (!rbac_system_)
    {
        printError("RBAC system not initialized");
        return -1;
    }
    if (rbac_system_->getCurrentRole() != "admin")
    {
        printError("Permission denied: Admin required");
        return -1;
    }
    if (cmd.args.size() < 3)
    {
        printError("Usage: useradd <username> <password> <role>");
        return -1;
    }
    if (rbac_system_->addUser(cmd.args[0], cmd.args[1], cmd.args[2]))
    {
        printSuccess("User added");
        return 0;
    }
    return -1;
}

int ShellState::cmdUserDel(const Command &cmd)
{
    if (!rbac_system_)
    {
        printError("RBAC system not initialized");
        return -1;
    }
    if (rbac_system_->getCurrentRole() != "admin")
    {
        printError("Permission denied: Admin required");
        return -1;
    }
    if (cmd.args.size() < 1)
    {
        printError("Usage: userdel <username>");
        return -1;
    }
    if (rbac_system_->deleteUser(cmd.args[0]))
    {
        printSuccess("User deleted");
        return 0;
    }
    return -1;
}

int ShellState::cmdPasswd(const Command &cmd)
{
    if (!rbac_system_)
    {
        printError("RBAC system not initialized");
        return -1;
    }
    if (cmd.args.size() < 2)
    {
        printError("Usage: passwd <username> <new_password>");
        return -1;
    }
    std::string target = cmd.args[0];
    if (rbac_system_->getCurrentRole() != "admin" && rbac_system_->getCurrentUser() != target)
    {
        printError("Permission denied: Admin or self required");
        return -1;
    }
    if (rbac_system_->setPassword(target, cmd.args[1]))
    {
        printSuccess("Password updated");
        return 0;
    }
    return -1;
}
