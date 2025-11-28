#ifndef SMART_SHELL_H
#define SMART_SHELL_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <atomic>

// Forward declarations
class ThemeManager;
class AutocompleteEngine;
class CommandDashboard;
#ifdef _WIN32
#define _WIN32_WINNT 0x0600
#include <windows.h>
// Windows: disable threading for MinGW compatibility
#ifndef NO_STD_THREAD
#define NO_STD_THREAD
#endif
#else
#define __GTHREAD_HAS_CONDITION
#include <pthread.h>
#include <thread>
#include <mutex>
#endif
#include <cstring>
#include <cstdlib>
#include <cctype>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <io.h>
#include <process.h>
#include <direct.h>
#define F_OK 0
#define access _access
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & _S_IFDIR) == _S_IFDIR)
#endif
#define getcwd _getcwd
#define chdir _chdir
#define isatty _isatty
typedef int pid_t;
#define WEXITSTATUS(status) (status)
#define waitpid(pid, status, options) 0
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#ifndef _WIN32
#include <curses.h>
#endif
#include <sys/statvfs.h>
#endif
#include <time.h>

// Constants
constexpr int MAX_COMMAND_LENGTH = 1024;
constexpr int MAX_ARGS = 64;
constexpr int MAX_HISTORY = 1000;
constexpr int MAX_SUGGESTIONS = 10;
constexpr int RESOURCE_CHECK_INTERVAL = 2;
constexpr int MAX_ROLES = 10;
constexpr int MAX_PERMISSIONS = 50;
constexpr int MAX_USERS = 100;

// Forward declarations
class ShellState;
class RBACSystem;
class ResourceMonitor;
class SuggestionEngine;
class UIManager;

// Command structure
struct Command
{
    std::string command;
    std::vector<std::string> args;
    bool background = false;
    std::string input_file;
    std::string output_file;
    bool append_output = false;
};

// Alias structure
struct Alias
{
    std::string alias_name;
    std::string command;
    int usage_count = 0;

    bool operator==(const std::string &name) const
    {
        return alias_name == name;
    }
};

// Resource monitoring structure
struct ResourceInfo
{
    double cpu_usage = 0.0;
    long memory_usage = 0;
    int process_count = 0;
    std::chrono::system_clock::time_point last_check;
};

// Suggestion structure
struct Suggestion
{
    std::string suggestion;
    std::string description;
    int confidence = 0;

    bool operator<(const Suggestion &other) const
    {
        return confidence > other.confidence; // Sort descending
    }
};

// Process info structure
struct ProcessInfo
{
    pid_t pid;
    std::string name;
    double cpu_percent;
    long memory_kb;
    char state;
};

// RBAC Permission definitions
namespace Permissions
{
    constexpr int READ_FILES = 1;
    constexpr int WRITE_FILES = 2;
    constexpr int EXECUTE_COMMANDS = 3;
    constexpr int SYSTEM_ADMIN = 4;
    constexpr int NETWORK_ACCESS = 5;
    constexpr int PROCESS_MANAGEMENT = 6;
    constexpr int USER_MANAGEMENT = 7;
    constexpr int ALIAS_MANAGEMENT = 8;
    constexpr int HISTORY_ACCESS = 9;
    constexpr int RESOURCE_MONITOR = 10;
}

// Permission structure
struct Permission
{
    std::string name;
    std::string description;
    int permission_id;
};

// Role structure
struct Role
{
    std::string name;
    std::string description;
    std::vector<int> permissions;

    bool hasPermission(int perm_id) const
    {
        return std::find(permissions.begin(), permissions.end(), perm_id) != permissions.end();
    }
};

// User structure
struct User
{
    std::string username;
    std::string role;
    std::string password;
    int user_id;
    int group_id;
    std::chrono::system_clock::time_point last_login;
    bool is_active;
};

// RBAC State
class RBACSystem
{
private:
    std::vector<Role> roles_;
    std::vector<User> users_;
    std::vector<Permission> permissions_;
    std::string current_user_;
    std::string current_role_;
#ifdef NO_STD_THREAD
    mutable int rbac_mutex_;
#else
    mutable std::mutex rbac_mutex_;
#endif

public:
    RBACSystem();
    bool initialize();
    bool checkPermission(const std::string &command, const std::string &username) const;
    bool addRole(const std::string &role_name, const std::string &description);
    bool addUser(const std::string &username, const std::string &password, const std::string &role);
    bool deleteUser(const std::string &username);
    bool setPassword(const std::string &username, const std::string &new_password);
    bool setUserRole(const std::string &username, const std::string &new_role);
    bool assignPermissionToRole(const std::string &role_name, int permission_id);
    bool loginUser(const std::string &username, const std::string &password);
    bool logoutUser();
    bool loadConfig();
    bool saveConfig() const;
    std::string getCurrentUser() const { return current_user_; }
    std::string getCurrentRole() const { return current_role_; }
    void listUsers() const;
    void listRoles() const;
    void listPermissions() const;
    bool isCommandAllowed(const std::string &command, const std::string &username) const;

private:
    const Role *findRole(const std::string &role_name) const;
    const User *findUser(const std::string &username) const;
    int getRequiredPermission(const std::string &command) const;
};

// Resource Monitor class
class ResourceMonitor
{
private:
#ifdef NO_STD_THREAD
    void *monitor_thread_;
#else
    std::unique_ptr<std::thread> monitor_thread_;
#endif
    std::atomic<bool> running_;
    ResourceInfo resources_;
#ifdef NO_STD_THREAD
    mutable int resource_mutex_;
#else
    mutable std::mutex resource_mutex_;
#endif

public:
    ResourceMonitor();
    ~ResourceMonitor();
    bool initialize();
    void stop();
    ResourceInfo getResources() const;
    bool checkWarning() const;
    void updateResources();
    void printSummary() const;

private:
    void monitorThread();
    double getCpuUsage();
    long getMemoryUsage();
    int getProcessCount();
    int getDiskUsage(const std::string &path) const;
    int getNetworkConnections() const;
    static unsigned long long prev_idle_;
    static unsigned long long prev_total_;
};

// Suggestion Engine class
class SuggestionEngine
{
private:
    struct CommonCommand
    {
        std::string command;
        std::string description;
        int confidence;
    };

    static const std::vector<CommonCommand> common_commands_;

public:
    std::vector<Suggestion> getSuggestions(const std::string &input) const;
    std::vector<Suggestion> findSimilarCommands(const std::string &input) const;
    std::vector<Suggestion> findHistorySuggestions(const std::string &input,
                                                   const std::vector<std::string> &history) const;
    std::vector<Suggestion> findCommonSuggestions(const std::string &input) const;
    std::string getCommandDescription(const std::string &command) const;
    bool shouldAutoAlias(const std::string &command, int usage_count) const;
    std::string generateAutoAlias(const std::string &command) const;
    bool suggestCorrection(const std::string &input, std::string &correction) const;
};

// UI Manager class
class UIManager
{
private:
#ifndef _WIN32
    WINDOW *main_win_;
    WINDOW *suggestion_win_;
    WINDOW *resource_win_;
#else
    void *main_win_;
    void *suggestion_win_;
    void *resource_win_;
#endif
    bool initialized_;

public:
    UIManager();
    ~UIManager();
    bool initialize();
    void cleanup();
    void displaySuggestions(const std::vector<Suggestion> &suggestions);
    void displayResources(const ResourceInfo &resources);
    void printMessage(const std::string &message, int type);
    void printPrompt(const std::string &prompt);
    void printOutput(const std::string &output);
    bool isInitialized() const { return initialized_; }
    void displayHelp();
    void displayResourceSummary(const ResourceInfo &resources);
    void displayHistory(const std::vector<std::string> &history, int count);
};

// Shell State class - Main shell implementation
class ShellState
{
private:
    std::string current_dir_;
    std::string home_dir_;
    std::vector<Alias> aliases_;
    std::vector<std::string> history_;
    std::unique_ptr<ResourceMonitor> resource_monitor_;
    std::unique_ptr<RBACSystem> rbac_system_;
    std::unique_ptr<UIManager> ui_manager_;
    std::unique_ptr<SuggestionEngine> suggestion_engine_;
    bool running_;
    bool interactive_;
    std::unordered_map<std::string, int> command_usage_;

    // New features
    class ThemeManager *theme_manager_;
    class AutocompleteEngine *autocomplete_engine_;
    class CommandDashboard *dashboard_;
#ifdef NO_STD_THREAD
    mutable int state_mutex_;
#else
    mutable std::mutex state_mutex_;
#endif

public:
    ShellState();
    ~ShellState();

    bool initialize();
    void cleanup();
    int run();

    // Command processing
    bool parseCommand(const std::string &input, Command &cmd);
    int executeCommand(const Command &cmd);
    int executeBuiltin(const Command &cmd);
    int executeExternal(const Command &cmd);

    // Alias management
    bool addAlias(const std::string &alias, const std::string &command);
    bool removeAlias(const std::string &alias);
    std::string expandAlias(const std::string &input);
    bool loadAliases();
    bool saveAliases() const;

    // History management
    void addToHistory(const std::string &command);
    std::vector<std::string> getHistory(int count = -1) const;
    std::vector<Alias> getAliases() const { return aliases_; }

    // Utility functions
    void printPrompt() const;
    bool isBuiltinCommand(const std::string &command) const;
    bool isShortcutCommand(const std::string &command) const;
    std::string expandShortcut(const std::string &input);

    // Getters
    std::string getCurrentDir() const { return current_dir_; }
    std::string getHomeDir() const { return home_dir_; }
    ResourceMonitor *getResourceMonitor() { return resource_monitor_.get(); }
    RBACSystem *getRBACSystem() { return rbac_system_.get(); }
    SuggestionEngine *getSuggestionEngine() { return suggestion_engine_.get(); }
    UIManager *getUIManager() { return ui_manager_.get(); }
    bool isRunning() const { return running_; }
    void setRunning(bool running) { running_ = running; }
    bool isInteractive() const { return interactive_; }
    void setInteractive(bool interactive) { interactive_ = interactive; }

    // Built-in command handlers (will be implemented in builtin_commands.cpp)
    int cmdCd(const Command &cmd);
    int cmdAlias(const Command &cmd);
    int cmdUnalias(const Command &cmd);
    int cmdHistory(const Command &cmd);
    int cmdHelp(const Command &cmd);
    int cmdExit(const Command &cmd);
    int cmdResources(const Command &cmd);
    int cmdSuggest(const Command &cmd);
    int cmdUsers(const Command &cmd);
    int cmdRoles(const Command &cmd);
    int cmdPermissions(const Command &cmd);
    int cmdLogin(const Command &cmd);
    int cmdLogout(const Command &cmd);
    int cmdSudo(const Command &cmd);
    int cmdUserAdd(const Command &cmd);
    int cmdUserDel(const Command &cmd);
    int cmdPasswd(const Command &cmd);
    int cmdClearHist(const Command &cmd);
};

// Utility functions
std::string trimWhitespace(const std::string &str);
bool fileExists(const std::string &filename);
bool isDirectory(const std::string &path);
void printError(const std::string &message);
void printSuccess(const std::string &message);
static inline std::string makeAliasFileForUser(const std::string &username)
{
    return ".smart_shell_aliases_" + username;
}

static inline std::string makeHistoryFileForUser(const std::string &username)
{
    return ".smart_shell_history_" + username;
}

#endif // SMART_SHELL_H
