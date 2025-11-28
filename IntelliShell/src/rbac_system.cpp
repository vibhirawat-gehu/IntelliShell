#include "smart_shell.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <iomanip>

RBACSystem::RBACSystem() : current_user_("anonymous"), current_role_("guest")
{
}

bool RBACSystem::initialize()
{
    // Initialize default permissions
    permissions_.clear();
    permissions_.push_back({"read_files", "Read files and directories", Permissions::READ_FILES});
    permissions_.push_back({"write_files", "Write and modify files", Permissions::WRITE_FILES});
    permissions_.push_back({"execute_commands", "Execute system commands", Permissions::EXECUTE_COMMANDS});
    permissions_.push_back({"system_admin", "System administration", Permissions::SYSTEM_ADMIN});
    permissions_.push_back({"network_access", "Network operations", Permissions::NETWORK_ACCESS});
    permissions_.push_back({"process_management", "Manage processes", Permissions::PROCESS_MANAGEMENT});
    permissions_.push_back({"user_management", "Manage users and roles", Permissions::USER_MANAGEMENT});
    permissions_.push_back({"alias_management", "Manage aliases", Permissions::ALIAS_MANAGEMENT});
    permissions_.push_back({"history_access", "Access command history", Permissions::HISTORY_ACCESS});
    permissions_.push_back({"resource_monitor", "Monitor system resources", Permissions::RESOURCE_MONITOR});

    // Initialize default roles only if they don't exist
    if (roles_.empty())
    {
        addRole("admin", "System Administrator");
        addRole("user", "Regular User");
        addRole("guest", "Guest User");
    }

    // Assign permissions to roles
    // Admin role - all permissions
    for (const auto &perm : permissions_)
    {
        assignPermissionToRole("admin", perm.permission_id);
    }

    // User role - almost full access (no system admin/user management)
    assignPermissionToRole("user", Permissions::READ_FILES);
    assignPermissionToRole("user", Permissions::WRITE_FILES);
    assignPermissionToRole("user", Permissions::EXECUTE_COMMANDS);
    assignPermissionToRole("user", Permissions::ALIAS_MANAGEMENT);
    assignPermissionToRole("user", Permissions::HISTORY_ACCESS);
    assignPermissionToRole("user", Permissions::RESOURCE_MONITOR);
    assignPermissionToRole("user", Permissions::NETWORK_ACCESS);
    assignPermissionToRole("user", Permissions::PROCESS_MANAGEMENT);

    // (Developer role removed)

    // Guest role - limited access: read, basic exec, aliases, history, resources
    assignPermissionToRole("guest", Permissions::READ_FILES);
    assignPermissionToRole("guest", Permissions::EXECUTE_COMMANDS);
    assignPermissionToRole("guest", Permissions::ALIAS_MANAGEMENT);
    assignPermissionToRole("guest", Permissions::HISTORY_ACCESS);
    assignPermissionToRole("guest", Permissions::RESOURCE_MONITOR);

    // Initialize default users only if they don't exist
    if (users_.empty())
    {
        addUser("admin", "admin@123", "admin");
        addUser("user", "user", "user");
        addUser("guest", "guest", "guest");
        // (Developer user removed)
    }

    // Set current user (default to system user)
    const char *current_user = getenv("USER");
    if (current_user)
    {
        current_user_ = current_user;
        // Try to find user in our system, default to 'user' role if not found
        const User *user = findUser(current_user_);
        if (user)
        {
            current_role_ = user->role;
        }
        else
        {
            current_role_ = "user";
        }
    }
    else
    {
        current_user_ = "anonymous";
        current_role_ = "guest";
    }

    // Load saved configuration
    loadConfig();

    return true;
}

bool RBACSystem::checkPermission(const std::string &command, const std::string &username) const
{
    return isCommandAllowed(command, username);
}

bool RBACSystem::addRole(const std::string &role_name, const std::string &description)
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    if (roles_.size() >= MAX_ROLES)
    {
        printError("Maximum number of roles reached");
        return false;
    }

    // Check if role already exists
    if (findRole(role_name))
    {
        printError("Role already exists");
        return false;
    }

    // Add new role
    Role new_role;
    new_role.name = role_name;
    new_role.description = description;
    new_role.permissions.clear();
    roles_.push_back(new_role);
    return true;
}

bool RBACSystem::addUser(const std::string &username, const std::string &password, const std::string &role)
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    if (users_.size() >= MAX_USERS)
    {
        printError("Maximum number of users reached");
        return false;
    }

    // Check if user already exists
    if (findUser(username))
    {
        printError("User already exists");
        return false;
    }

    // Verify role exists
    if (!findRole(role))
    {
        printError("Role does not exist");
        return false;
    }

    // Add new user
    User new_user;
    new_user.username = username;
    new_user.role = role;
    new_user.password = password;
    new_user.user_id = static_cast<int>(users_.size()) + 1000;
    new_user.group_id = 100;
    new_user.last_login = std::chrono::system_clock::now();
    new_user.is_active = true;
    users_.push_back(new_user);
    return true;
}

bool RBACSystem::deleteUser(const std::string &username)
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    for (auto it = users_.begin(); it != users_.end(); ++it)
    {
        if (it->username == username)
        {
            // Prevent deleting admin account
            if (username == "admin")
            {
                printError("Cannot delete built-in admin user");
                return false;
            }
            // Remove per-user files: aliases and history
            std::string alias_file = makeAliasFileForUser(username);
            std::string hist_file = makeHistoryFileForUser(username);
            std::remove(alias_file.c_str());
            std::remove(hist_file.c_str());

            users_.erase(it);
            return true;
        }
    }
    printError("User not found");
    return false;
}

bool RBACSystem::setPassword(const std::string &username, const std::string &new_password)
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    for (auto &u : users_)
    {
        if (u.username == username)
        {
            u.password = new_password;
            return true;
        }
    }
    printError("User not found");
    return false;
}

bool RBACSystem::setUserRole(const std::string &username, const std::string &new_role)
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    if (!findRole(new_role))
    {
        printError("Role does not exist");
        return false;
    }

    for (auto &u : users_)
    {
        if (u.username == username)
        {
            u.role = new_role;
            return true;
        }
    }
    printError("User not found");
    return false;
}

bool RBACSystem::assignPermissionToRole(const std::string &role_name, int permission_id)
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    Role *role = nullptr;
    for (auto &r : roles_)
    {
        if (r.name == role_name)
        {
            role = &r;
            break;
        }
    }

    if (!role)
    {
        printError("Role not found");
        return false;
    }

    // Check if permission already assigned
    if (role->hasPermission(permission_id))
    {
        return true; // Already assigned
    }

    // Add permission
    if (role->permissions.size() >= MAX_PERMISSIONS)
    {
        printError("Maximum permissions per role reached");
        return false;
    }

    role->permissions.push_back(permission_id);
    return true;
}

bool RBACSystem::loginUser(const std::string &username, const std::string &password)
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    const User *user = findUser(username);
    if (!user)
    {
        printError("User not found");
        return false;
    }

    if (user->password != password)
    {
        printError("Invalid credentials");
        return false;
    }

    if (!user->is_active)
    {
        printError("User account is disabled");
        return false;
    }

    // Update login info (non-const access needed)
    for (auto &u : users_)
    {
        if (u.username == username)
        {
            u.last_login = std::chrono::system_clock::now();
            break;
        }
    }

    current_user_ = username;
    current_role_ = user->role;

    std::cout << "Logged in as " << username << " (" << user->role << ")" << std::endl;
    return true;
}

bool RBACSystem::logoutUser()
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    std::cout << "Logged out from " << current_user_ << std::endl;
    current_user_ = "anonymous";
    current_role_ = "guest";
    return true;
}

bool RBACSystem::loadConfig()
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    std::ifstream file(".smart_shell_rbac");
    if (!file.is_open())
    {
        return true; // No config file, use defaults
    }

    std::string line;
    while (std::getline(file, line))
    {
        line = trimWhitespace(line);

        if (line.substr(0, 5) == "USER:")
        {
            // Load user configuration
            std::istringstream iss(line.substr(5));
            std::string username, role, password;
            if (iss >> username >> role >> password)
            {
                addUser(username, password, role);
            }
        }
        else if (line.substr(0, 5) == "ROLE:")
        {
            // Load role configuration
            std::istringstream iss(line.substr(5));
            std::string role_name;
            std::string description;
            if (iss >> role_name)
            {
                std::getline(iss, description);
                description = trimWhitespace(description);
                addRole(role_name, description);
            }
        }
    }

    return true;
}

bool RBACSystem::saveConfig() const
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    std::ofstream file(".smart_shell_rbac");
    if (!file.is_open())
    {
        printError("Failed to save RBAC configuration");
        return false;
    }

    // Save users
    for (const auto &user : users_)
    {
        file << "USER:" << user.username << " " << user.role << " " << user.password << std::endl;
    }

    // Save roles
    for (const auto &role : roles_)
    {
        file << "ROLE:" << role.name << " " << role.description << std::endl;
    }

    return true;
}

void RBACSystem::listUsers() const
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    std::cout << "Users in system:" << std::endl;
    std::cout << std::left << std::setw(15) << "Username"
              << std::setw(15) << "Role"
              << std::setw(10) << "Status"
              << std::setw(20) << "Last Login"
              << "User ID" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (const auto &user : users_)
    {
        std::string status = user.is_active ? "Active" : "Disabled";
        auto time_t = std::chrono::system_clock::to_time_t(user.last_login);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");

        std::cout << std::left << std::setw(15) << user.username
                  << std::setw(15) << user.role
                  << std::setw(10) << status
                  << std::setw(20) << ss.str()
                  << user.user_id << std::endl;
    }
}

void RBACSystem::listRoles() const
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    std::cout << "Roles in system:" << std::endl;
    std::cout << std::left << std::setw(15) << "Role"
              << std::setw(30) << "Description"
              << "Permissions" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (const auto &role : roles_)
    {
        std::cout << std::left << std::setw(15) << role.name
                  << std::setw(30) << role.description;

        // List permissions
        for (size_t i = 0; i < role.permissions.size(); i++)
        {
            int perm_id = role.permissions[i];
            for (const auto &perm : permissions_)
            {
                if (perm.permission_id == perm_id)
                {
                    std::cout << perm.name;
                    if (i < role.permissions.size() - 1)
                    {
                        std::cout << ", ";
                    }
                    break;
                }
            }
        }
        std::cout << std::endl;
    }
}

void RBACSystem::listPermissions() const
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    std::cout << "Available permissions:" << std::endl;
    std::cout << std::left << std::setw(20) << "Permission"
              << "Description" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (const auto &perm : permissions_)
    {
        std::cout << std::left << std::setw(20) << perm.name
                  << perm.description << std::endl;
    }
}

bool RBACSystem::isCommandAllowed(const std::string &command, const std::string &username) const
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(rbac_mutex_);
#endif

    // Basic commands allowed for everyone (including anonymous users)
    static const std::vector<std::string> basic_commands = {
        "help", "exit", "ls", "dir", "cd", "pwd", "cat", "type",
        "echo", "clear", "cls", "whoami", "date", "time", "history",
        "suggest", "resources", "login", "logout"};

    for (const auto &cmd : basic_commands)
    {
        if (command == cmd)
        {
            return true; // Allow basic commands for everyone
        }
    }

    const User *user = findUser(username);
    const Role *role = nullptr;
    if (!user)
    {
        // Treat anonymous as guest role
        if (username == "anonymous")
        {
            role = findRole("guest");
        }
        else
        {
            return false; // Unknown user rejected (basic commands already allowed)
        }
    }
    else
    {
        role = findRole(user->role);
    }
    if (!role)
    {
        return false;
    }

    // Check permissions based on command
    int required_permission = getRequiredPermission(command);

    return role->hasPermission(required_permission);
}

const Role *RBACSystem::findRole(const std::string &role_name) const
{
    for (const auto &role : roles_)
    {
        if (role.name == role_name)
        {
            return &role;
        }
    }
    return nullptr;
}

const User *RBACSystem::findUser(const std::string &username) const
{
    for (const auto &user : users_)
    {
        if (user.username == username)
        {
            return &user;
        }
    }
    return nullptr;
}

int RBACSystem::getRequiredPermission(const std::string &command) const
{
    // Map commands to permissions
    // Basic commands don't require special permissions (handled above)
    if (command == "ls" || command == "cat" || command == "pwd" || command == "dir")
    {
        return Permissions::READ_FILES;
    }
    else if (command == "rm" || command == "cp" || command == "mv" ||
             command == "mkdir" || command == "rmdir")
    {
        return Permissions::WRITE_FILES;
    }
    else if (command == "ps" || command == "kill" || command == "top")
    {
        return Permissions::PROCESS_MANAGEMENT;
    }
    else if (command == "ping" || command == "netstat" || command == "wget")
    {
        return Permissions::NETWORK_ACCESS;
    }
    else if (command == "alias" || command == "unalias")
    {
        return Permissions::ALIAS_MANAGEMENT;
    }
    else if (command == "useradd" || command == "userdel" || command == "passwd")
    {
        return Permissions::USER_MANAGEMENT;
    }
    else if (command == "history")
    {
        return Permissions::HISTORY_ACCESS;
    }
    else if (command == "resources")
    {
        return Permissions::RESOURCE_MONITOR;
    }
    else if (command == "users")
    {
        // Viewing users restricted to admin
        return Permissions::USER_MANAGEMENT;
    }
    else if (command == "roles" || command == "permissions")
    {
        // Keep roles/permissions visible to all
        return Permissions::READ_FILES;
    }
    else if (command == "sudo")
    {
        return Permissions::SYSTEM_ADMIN;
    }
    else if (command == "clearhist")
    {
        return Permissions::SYSTEM_ADMIN;
    }
    else
    {
        // Default to execute permission for unknown commands
        return Permissions::EXECUTE_COMMANDS;
    }
}