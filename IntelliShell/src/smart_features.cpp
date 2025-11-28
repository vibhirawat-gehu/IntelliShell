#include "smart_features.h"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <thread>
#else
#include <unistd.h>
#endif

// ============================================================================
// SmartPredictor Implementation
// ============================================================================

SmartPredictor::SmartPredictor() {}
SmartPredictor::~SmartPredictor() {}

void SmartPredictor::learnCommand(const std::string &command, const std::string &directory)
{
    command_frequency_[command]++;

    // Learn command sequences
    if (!command_history_.empty())
    {
        std::string last_cmd = command_history_.back().command;
        command_sequences_[last_cmd].push_back(command);
    }

    // Store context
    CommandContext ctx;
    ctx.command = command;
    ctx.directory = directory;
    ctx.last_used = std::chrono::system_clock::now();

    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    ctx.hour_of_day = ltm->tm_hour;
    ctx.day_of_week = ltm->tm_wday;
    ctx.frequency = command_frequency_[command];

    command_history_.push_back(ctx);
}

std::vector<std::string> SmartPredictor::getPredictions(const std::string &current_dir, const std::string &partial_input)
{
    std::vector<std::string> predictions;

    // Get current time context
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    int current_hour = ltm->tm_hour;

    // Find commands matching partial input and context
    for (const auto &ctx : command_history_)
    {
        if (ctx.command.find(partial_input) == 0)
        {
            // Boost score if same directory or similar time
            bool same_dir = (ctx.directory == current_dir);
            bool similar_time = (abs(ctx.hour_of_day - current_hour) < 2);

            if (same_dir || similar_time || ctx.frequency > 5)
            {
                if (std::find(predictions.begin(), predictions.end(), ctx.command) == predictions.end())
                {
                    predictions.push_back(ctx.command);
                }
            }
        }
    }

    return predictions;
}

std::vector<std::string> SmartPredictor::getNextCommandSuggestions(const std::string &last_command)
{
    if (command_sequences_.find(last_command) != command_sequences_.end())
    {
        return command_sequences_[last_command];
    }
    return {};
}

std::string SmartPredictor::getMostUsedCommand()
{
    std::string most_used;
    int max_freq = 0;

    for (const auto &pair : command_frequency_)
    {
        if (pair.second > max_freq)
        {
            max_freq = pair.second;
            most_used = pair.first;
        }
    }

    return most_used;
}

// ============================================================================
// AdaptiveUI Implementation
// ============================================================================

AdaptiveUI::AdaptiveUI()
    : show_hints_(true), compact_mode_(false), user_skill_level_(0), commands_executed_(0) {}

AdaptiveUI::~AdaptiveUI() {}

void AdaptiveUI::updateSkillLevel(bool command_successful)
{
    commands_executed_++;

    // Adjust skill level based on usage
    if (commands_executed_ > 100 && user_skill_level_ == 0)
    {
        user_skill_level_ = 1; // Intermediate
        show_hints_ = false;   // Reduce hints for intermediate users
    }
    else if (commands_executed_ > 500 && user_skill_level_ == 1)
    {
        user_skill_level_ = 2; // Advanced
        compact_mode_ = true;  // More compact UI for advanced users
    }
}

std::string AdaptiveUI::getContextualHelp(const std::string &failed_command)
{
    error_count_[failed_command]++;

    if (failed_command.find("cd") == 0)
    {
        return "Tip: Use 'cd <directory>' to change directory. Try 'cd ..' to go up one level.";
    }
    else if (failed_command.find("ls") == 0 || failed_command.find("dir") == 0)
    {
        return "Tip: Use 'll' for detailed listing or 'la' to show hidden files.";
    }
    else if (failed_command.find("rm") == 0 || failed_command.find("del") == 0)
    {
        return "Warning: Be careful with delete commands! They cannot be undone.";
    }

    return "Type 'help' to see available commands, or use Tab for autocomplete.";
}

void AdaptiveUI::showSmartNotification(const std::string &message, const std::string &type)
{
    if (type == "success")
    {
        std::cout << "\033[42m\033[1;37m SUCCESS \033[0m \033[32m" << message << "\033[0m\n";
    }
    else if (type == "error")
    {
        std::cout << "\033[41m\033[1;37m ERROR \033[0m \033[31m" << message << "\033[0m\n";
    }
    else if (type == "warning")
    {
        std::cout << "\033[43m\033[1;30m WARNING \033[0m \033[33m" << message << "\033[0m\n";
    }
    else if (type == "info")
    {
        std::cout << "\033[44m\033[1;37m INFO \033[0m \033[36m" << message << "\033[0m\n";
    }
}

// ============================================================================
// ResourceAwareExecutor Implementation
// ============================================================================

ResourceAwareExecutor::ResourceAwareExecutor()
    : cpu_threshold_(80.0), memory_threshold_(85.0), auto_optimize_(true) {}

ResourceAwareExecutor::~ResourceAwareExecutor() {}

bool ResourceAwareExecutor::canExecuteHeavyCommand(double current_cpu, double current_memory)
{
    return (current_cpu < cpu_threshold_ && current_memory < memory_threshold_);
}

std::string ResourceAwareExecutor::suggestOptimalTime(const std::string &command)
{
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    int hour = ltm->tm_hour;

    if (hour >= 9 && hour <= 17)
    {
        return "Consider running this during off-peak hours (evening/night) for better performance.";
    }

    return "Current time is optimal for resource-intensive operations.";
}

std::string ResourceAwareExecutor::optimizeCommand(const std::string &command, double current_cpu, double current_memory)
{
    if (!auto_optimize_)
        return command;

    // Add nice/priority adjustments for heavy commands
    if (current_cpu > 70.0)
    {
        return "nice -n 10 " + command; // Lower priority
    }

    return command;
}

// ============================================================================
// SmartCorrector Implementation
// ============================================================================

SmartCorrector::SmartCorrector()
{
    // Initialize common typos
    common_typos_["hlep"] = "help";
    common_typos_["exti"] = "exit";
    common_typos_["cler"] = "clear";
    common_typos_["histroy"] = "history";
}

SmartCorrector::~SmartCorrector() {}

int SmartCorrector::calculateDistance(const std::string &s1, const std::string &s2)
{
    int len1 = s1.length();
    int len2 = s2.length();
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

    for (int i = 0; i <= len1; i++)
        dp[i][0] = i;
    for (int j = 0; j <= len2; j++)
        dp[0][j] = j;

    for (int i = 1; i <= len1; i++)
    {
        for (int j = 1; j <= len2; j++)
        {
            if (s1[i - 1] == s2[j - 1])
            {
                dp[i][j] = dp[i - 1][j - 1];
            }
            else
            {
                dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }

    return dp[len1][len2];
}

std::vector<std::string> SmartCorrector::suggestCorrections(const std::string &input)
{
    std::vector<std::string> suggestions;

    // Check common typos first
    if (common_typos_.find(input) != common_typos_.end())
    {
        suggestions.push_back(common_typos_[input]);
        return suggestions;
    }

    // Find similar commands
    for (const auto &cmd : available_commands_)
    {
        int distance = calculateDistance(input, cmd);
        if (distance <= 2) // Allow up to 2 character differences
        {
            suggestions.push_back(cmd);
        }
    }

    return suggestions;
}

void SmartCorrector::addCommand(const std::string &command)
{
    if (std::find(available_commands_.begin(), available_commands_.end(), command) == available_commands_.end())
    {
        available_commands_.push_back(command);
    }
}

void SmartCorrector::learnCorrection(const std::string &wrong, const std::string &correct)
{
    common_typos_[wrong] = correct;
}

// ============================================================================
// VisualEnhancer Implementation
// ============================================================================

VisualEnhancer::VisualEnhancer()
    : animations_enabled_(true), colors_enabled_(true), current_theme_("default") {}

VisualEnhancer::~VisualEnhancer() {}

void VisualEnhancer::showProgressBar(int percentage, const std::string &label)
{
    if (!animations_enabled_)
        return;

    int bar_width = 50;
    int filled = (percentage * bar_width) / 100;

    std::cout << "\r\033[36m" << label << " \033[0m[";

    for (int i = 0; i < bar_width; i++)
    {
        if (i < filled)
        {
            std::cout << "\033[32m#\033[0m";
        }
        else
        {
            std::cout << "\033[90m-\033[0m";
        }
    }

    std::cout << "] " << percentage << "%";
    std::cout.flush();

    if (percentage >= 100)
    {
        std::cout << " \033[32mDONE!\033[0m\n";
    }
}

void VisualEnhancer::showLoadingAnimation(const std::string &message)
{
    if (!animations_enabled_)
        return;

    const char spinner[] = {'|', '/', '-', '\\'};
    for (int i = 0; i < 4; i++)
    {
        std::cout << "\r\033[36m" << spinner[i] << " " << message << "...\033[0m";
        std::cout.flush();
#ifdef _WIN32
        Sleep(200);
#else
        usleep(200000);
#endif
    }
    std::cout << "\r\033[K"; // Clear line
}

void VisualEnhancer::showSuccess(const std::string &message)
{
    std::cout << "\n";
    std::cout << "  +----------------------------------------------------------+\n";
    std::cout << "  |  \033[42m\033[1;37m SUCCESS \033[0m  " << std::left << std::setw(42) << message << " |\n";
    std::cout << "  +----------------------------------------------------------+\n";
    std::cout << "\n";
}

void VisualEnhancer::showError(const std::string &message)
{
    std::cout << "\n";
    std::cout << "  +----------------------------------------------------------+\n";
    std::cout << "  |  \033[41m\033[1;37m ERROR \033[0m    " << std::left << std::setw(42) << message << " |\n";
    std::cout << "  +----------------------------------------------------------+\n";
    std::cout << "\n";
}

void VisualEnhancer::showWarning(const std::string &message)
{
    std::cout << "\n";
    std::cout << "  +----------------------------------------------------------+\n";
    std::cout << "  |  \033[43m\033[1;30m WARNING \033[0m  " << std::left << std::setw(42) << message << " |\n";
    std::cout << "  +----------------------------------------------------------+\n";
    std::cout << "\n";
}

void VisualEnhancer::showInfo(const std::string &message)
{
    std::cout << "\n";
    std::cout << "  +----------------------------------------------------------+\n";
    std::cout << "  |  \033[44m\033[1;37m INFO \033[0m     " << std::left << std::setw(42) << message << " |\n";
    std::cout << "  +----------------------------------------------------------+\n";
    std::cout << "\n";
}

void VisualEnhancer::drawBox(const std::string &title, const std::vector<std::string> &content)
{
    int max_width = title.length();
    for (const auto &line : content)
    {
        if ((int)line.length() > max_width)
            max_width = line.length();
    }

    max_width += 4; // Padding

    // Top border
    std::cout << "\n  +";
    for (int i = 0; i < max_width; i++)
        std::cout << "-";
    std::cout << "+\n";

    // Title
    std::cout << "  | \033[1;36m" << std::left << std::setw(max_width - 2) << title << "\033[0m |\n";

    // Separator
    std::cout << "  +";
    for (int i = 0; i < max_width; i++)
        std::cout << "-";
    std::cout << "+\n";

    // Content
    for (const auto &line : content)
    {
        std::cout << "  | " << std::left << std::setw(max_width - 2) << line << " |\n";
    }

    // Bottom border
    std::cout << "  +";
    for (int i = 0; i < max_width; i++)
        std::cout << "-";
    std::cout << "+\n\n";
}

void VisualEnhancer::fadeIn()
{
    if (!animations_enabled_)
        return;

    for (int i = 0; i < 10; i++)
    {
        std::cout << "\033[2J\033[H"; // Clear screen
#ifdef _WIN32
        Sleep(50);
#else
        usleep(50000);
#endif
    }
}

void VisualEnhancer::fadeOut()
{
    if (!animations_enabled_)
        return;

    for (int i = 0; i < 10; i++)
    {
#ifdef _WIN32
        Sleep(50);
#else
        usleep(50000);
#endif
    }
    std::cout << "\033[2J\033[H"; // Clear screen
}

// ============================================================================
// SmartClipboard Implementation
// ============================================================================

SmartClipboard::SmartClipboard() : max_history_(10) {}
SmartClipboard::~SmartClipboard() {}

void SmartClipboard::copy(const std::string &text)
{
    clipboard_history_.insert(clipboard_history_.begin(), text);
    if ((int)clipboard_history_.size() > max_history_)
    {
        clipboard_history_.pop_back();
    }
}

std::string SmartClipboard::paste()
{
    if (!clipboard_history_.empty())
    {
        return clipboard_history_[0];
    }
    return "";
}

void SmartClipboard::clearHistory()
{
    clipboard_history_.clear();
}

// ============================================================================
// CommandBuilder Implementation
// ============================================================================

CommandBuilder::CommandBuilder() {}
CommandBuilder::~CommandBuilder() {}

std::string CommandBuilder::buildCommand(const std::string &base_command)
{
    // Interactive command building would go here
    return base_command;
}

std::vector<std::string> CommandBuilder::getParameterSuggestions(const std::string &command)
{
    std::vector<std::string> suggestions;

    if (command == "cd")
    {
        suggestions = {"..", "~", "/", "Documents", "Downloads"};
    }
    else if (command == "ls" || command == "dir")
    {
        suggestions = {"-l", "-a", "-la", "-lh", "-R"};
    }
    else if (command == "find" || command == "findf")
    {
        suggestions = {"*.txt", "*.cpp", "*.h", "*.py", "*.js"};
    }

    return suggestions;
}

bool CommandBuilder::validateCommand(const std::string &command, std::string &error_message)
{
    if (command.empty())
    {
        error_message = "Command cannot be empty";
        return false;
    }

    // Add more validation rules here

    return true;
}
