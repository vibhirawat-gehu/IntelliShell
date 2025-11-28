#include "smart_shell.h"
#include <algorithm>

// Static common commands database
const std::vector<SuggestionEngine::CommonCommand> SuggestionEngine::common_commands_ = {
    {"ls", "List directory contents", 90},
    {"cd", "Change directory", 90},
    {"pwd", "Print working directory", 85},
    {"help", "Show help for available commands", 90},
    {"alias", "Create or list command aliases", 85},
    {"unalias", "Remove an alias by name", 85},
    {"history", "Show previously executed commands", 85},
    {"login", "Login: login <username> <password>", 90},
    {"logout", "Logout current user", 90},
    {"users", "List system users", 70},
    {"roles", "List system roles", 70},
    {"permissions", "List available permissions", 70},
    {"sudo", "Execute a command with admin privileges", 70},
    {"useradd", "Create a new user: useradd <u> <p> <role>", 70},
    {"userdel", "Delete a user: userdel <u>", 70},
    {"passwd", "Set password: passwd <u> <p>", 70},
    {"clearhist", "Delete per-user history files (admin)", 60},
    {"resources", "Show system resource usage", 70},
    {"suggest", "Suggest commands for a partial prefix", 70},
    {"mkdir", "Create directory", 85},
    {"rmdir", "Remove directory", 85},
    {"rm", "Remove files", 85},
    {"cp", "Copy files", 85},
    {"mv", "Move/rename files", 85},
    {"cat", "Display file contents", 80},
    {"grep", "Search text in files", 80},
    {"find", "Find files", 80},
    {"ps", "List processes", 80},
    {"top", "Display running processes", 80},
    {"kill", "Terminate processes", 80},
    {"chmod", "Change file permissions", 75},
    {"chown", "Change file ownership", 75},
    {"tar", "Archive files", 75},
    {"gzip", "Compress files", 75},
    {"ssh", "Secure shell connection", 75},
    {"scp", "Secure copy", 75},
    {"wget", "Download files", 70},
    {"curl", "Transfer data", 70},
    {"git", "Version control", 70},
    {"make", "Build system", 70},
    {"gcc", "C compiler", 70},
    {"python", "Python interpreter", 70},
    {"node", "Node.js runtime", 70},
    {"npm", "Node package manager", 70},
    {"docker", "Container platform", 65},
    {"kubectl", "Kubernetes CLI", 65}
};

std::vector<Suggestion> SuggestionEngine::getSuggestions(const std::string& input) const {
    std::vector<Suggestion> suggestions;
    
    // Get suggestions from similar commands
    auto similar = findSimilarCommands(input);
    suggestions.insert(suggestions.end(), similar.begin(), similar.end());
    
    // Get suggestions from common commands
    auto common = findCommonSuggestions(input);
    suggestions.insert(suggestions.end(), common.begin(), common.end());
    
    // Sort suggestions by confidence
    std::sort(suggestions.begin(), suggestions.end());
    
    // Remove duplicates
    auto last = std::unique(suggestions.begin(), suggestions.end(),
                          [](const Suggestion& a, const Suggestion& b) {
                              return a.suggestion == b.suggestion;
                          });
    suggestions.erase(last, suggestions.end());
    
    // Limit to MAX_SUGGESTIONS
    if (suggestions.size() > MAX_SUGGESTIONS) {
        suggestions.resize(MAX_SUGGESTIONS);
    }
    
    return suggestions;
}

std::vector<Suggestion> SuggestionEngine::findSimilarCommands(const std::string& input) const {
    std::vector<Suggestion> suggestions;
    
    for (const auto& cmd : common_commands_) {
        if (cmd.command.find(input) != std::string::npos || 
            input.find(cmd.command) != std::string::npos) {
            Suggestion sug;
            sug.suggestion = cmd.command;
            sug.description = cmd.description;
            sug.confidence = cmd.confidence;
            suggestions.push_back(sug);
        }
    }
    
    return suggestions;
}

std::vector<Suggestion> SuggestionEngine::findHistorySuggestions(
    const std::string& input, 
    const std::vector<std::string>& history) const {
    std::vector<Suggestion> suggestions;
    
    for (const auto& cmd : history) {
        if (cmd.find(input) != std::string::npos) {
            Suggestion sug;
            sug.suggestion = cmd;
            sug.description = "From history";
            sug.confidence = 60;
            suggestions.push_back(sug);
        }
    }
    
    return suggestions;
}

std::vector<Suggestion> SuggestionEngine::findCommonSuggestions(const std::string& input) const {
    std::vector<Suggestion> suggestions;
    std::string lower_input = input;
    std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), ::tolower);
    
    // File operation suggestions
    if (lower_input.find("ls") != std::string::npos || lower_input.find("list") != std::string::npos) {
        Suggestion sug;
        sug.suggestion = "ls -la";
        sug.description = "List all files with details";
        sug.confidence = 85;
        suggestions.push_back(sug);
    }
    
    if (lower_input.find("find") != std::string::npos) {
        Suggestion sug;
        sug.suggestion = "find . -name \"*.txt\"";
        sug.description = "Find text files in current directory";
        sug.confidence = 80;
        suggestions.push_back(sug);
    }
    
    if (lower_input.find("grep") != std::string::npos) {
        Suggestion sug;
        sug.suggestion = "grep -r \"pattern\" .";
        sug.description = "Search for pattern recursively";
        sug.confidence = 80;
        suggestions.push_back(sug);
    }
    
    // Process management suggestions
    if (lower_input.find("ps") != std::string::npos || lower_input.find("process") != std::string::npos) {
        Suggestion sug;
        sug.suggestion = "ps aux";
        sug.description = "List all processes";
        sug.confidence = 85;
        suggestions.push_back(sug);
    }
    
    if (lower_input.find("kill") != std::string::npos) {
        Suggestion sug;
        sug.suggestion = "kill -9 <pid>";
        sug.description = "Force kill process";
        sug.confidence = 75;
        suggestions.push_back(sug);
    }
    
    // Network suggestions
    if (lower_input.find("net") != std::string::npos || lower_input.find("network") != std::string::npos) {
        Suggestion sug;
        sug.suggestion = "netstat -tuln";
        sug.description = "Show network connections";
        sug.confidence = 80;
        suggestions.push_back(sug);
    }
    
    if (lower_input.find("ping") != std::string::npos) {
        Suggestion sug;
        sug.suggestion = "ping google.com";
        sug.description = "Test network connectivity";
        sug.confidence = 85;
        suggestions.push_back(sug);
    }
    
    // System information suggestions
    if (lower_input.find("disk") != std::string::npos || lower_input.find("space") != std::string::npos) {
        Suggestion sug;
        sug.suggestion = "df -h";
        sug.description = "Show disk space usage";
        sug.confidence = 85;
        suggestions.push_back(sug);
    }
    
    if (lower_input.find("memory") != std::string::npos || lower_input.find("ram") != std::string::npos) {
        Suggestion sug;
        sug.suggestion = "free -h";
        sug.description = "Show memory usage";
        sug.confidence = 85;
        suggestions.push_back(sug);
    }
    
    return suggestions;
}
std::string SuggestionEngine::getCommandDescription(const std::string& command) const {
    for (const auto& cmd : common_commands_) {
        if (cmd.command == command) {
            return cmd.description;
        }
    }
    return "";
}

bool SuggestionEngine::shouldAutoAlias(const std::string& command, int usage_count) const {
    // Auto-alias commands used more than 5 times
    return usage_count > 5;
}

std::string SuggestionEngine::generateAutoAlias(const std::string& command) const {
    // Simple auto-alias generation based on command patterns
    if (command.find("ls -la") != std::string::npos) {
        return "ll";
    }
    
    if (command.find("ps aux") != std::string::npos) {
        return "ps";
    }
    
    if (command.find("netstat -tuln") != std::string::npos) {
        return "net";
    }
    
    if (command.find("find . -name") != std::string::npos) {
        return "findf";
    }
    
    return "";
}

bool SuggestionEngine::suggestCorrection(const std::string& input, std::string& correction) const {
    // Common typos and corrections
    static const std::map<std::string, std::string> corrections = {
        {"sl", "ls"},
        {"cd..", "cd .."},
        {"makedir", "mkdir"},
        {"removedir", "rmdir"},
        {"copy", "cp"},
        {"move", "mv"},
        {"type", "cat"},
        {"dir", "ls"},
        {"del", "rm"},
        {"ren", "mv"},
        {"cls", "clear"}
    };
    
    auto it = corrections.find(input);
    if (it != corrections.end()) {
        correction = it->second;
        return true;
    }
    
    return false;
}



