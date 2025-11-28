#ifndef SMART_FEATURES_H
#define SMART_FEATURES_H

#include <string>
#include <vector>
#include <map>
#include <chrono>

// Smart command prediction based on context and time
class SmartPredictor
{
private:
    struct CommandContext
    {
        std::string command;
        std::string directory;
        int hour_of_day;
        int day_of_week;
        int frequency;
        std::chrono::system_clock::time_point last_used;
    };

    std::vector<CommandContext> command_history_;
    std::map<std::string, int> command_frequency_;
    std::map<std::string, std::vector<std::string>> command_sequences_; // What usually follows what

public:
    SmartPredictor();
    ~SmartPredictor();

    // Learn from user behavior
    void learnCommand(const std::string &command, const std::string &directory);

    // Get smart predictions based on context
    std::vector<std::string> getPredictions(const std::string &current_dir, const std::string &partial_input);

    // Get next likely command based on previous command
    std::vector<std::string> getNextCommandSuggestions(const std::string &last_command);

    // Analyze patterns
    std::string getMostUsedCommand();
    std::string getMostUsedCommandAtTime(int hour);
};

// Adaptive UI that changes based on user behavior
class AdaptiveUI
{
private:
    bool show_hints_;
    bool compact_mode_;
    int user_skill_level_; // 0=beginner, 1=intermediate, 2=advanced
    int commands_executed_;
    std::map<std::string, int> error_count_;

public:
    AdaptiveUI();
    ~AdaptiveUI();

    // Adjust UI based on user behavior
    void updateSkillLevel(bool command_successful);
    int getSkillLevel() const { return user_skill_level_; }

    // Show contextual hints
    bool shouldShowHints() const { return show_hints_; }
    void setShowHints(bool show) { show_hints_ = show; }

    // Get contextual help message
    std::string getContextualHelp(const std::string &failed_command);

    // Display smart notification
    void showSmartNotification(const std::string &message, const std::string &type);
};

// Resource-aware command execution
class ResourceAwareExecutor
{
private:
    double cpu_threshold_;
    double memory_threshold_;
    bool auto_optimize_;

public:
    ResourceAwareExecutor();
    ~ResourceAwareExecutor();

    // Check if it's safe to run resource-intensive command
    bool canExecuteHeavyCommand(double current_cpu, double current_memory);

    // Suggest optimal time to run command
    std::string suggestOptimalTime(const std::string &command);

    // Auto-optimize command based on resources
    std::string optimizeCommand(const std::string &command, double current_cpu, double current_memory);

    // Set thresholds
    void setCPUThreshold(double threshold) { cpu_threshold_ = threshold; }
    void setMemoryThreshold(double threshold) { memory_threshold_ = threshold; }
};

// Smart command corrections and suggestions
class SmartCorrector
{
private:
    std::map<std::string, std::string> common_typos_;
    std::vector<std::string> available_commands_;

public:
    SmartCorrector();
    ~SmartCorrector();

    // Calculate string similarity (Levenshtein distance)
    int calculateDistance(const std::string &s1, const std::string &s2);

    // Suggest corrections for mistyped commands
    std::vector<std::string> suggestCorrections(const std::string &input);

    // Add available commands
    void addCommand(const std::string &command);

    // Learn from corrections
    void learnCorrection(const std::string &wrong, const std::string &correct);
};

// Visual enhancements and animations
class VisualEnhancer
{
private:
    bool animations_enabled_;
    bool colors_enabled_;
    std::string current_theme_;

public:
    VisualEnhancer();
    ~VisualEnhancer();

    // Display animated progress bar
    void showProgressBar(int percentage, const std::string &label);

    // Display loading animation
    void showLoadingAnimation(const std::string &message);

    // Display success/error with visual feedback
    void showSuccess(const std::string &message);
    void showError(const std::string &message);
    void showWarning(const std::string &message);
    void showInfo(const std::string &message);

    // Draw fancy box around text
    void drawBox(const std::string &title, const std::vector<std::string> &content);

    // Smooth transitions
    void fadeIn();
    void fadeOut();

    // Enable/disable features
    void setAnimations(bool enabled) { animations_enabled_ = enabled; }
    void setColors(bool enabled) { colors_enabled_ = enabled; }
};

// Smart clipboard manager
class SmartClipboard
{
private:
    std::vector<std::string> clipboard_history_;
    int max_history_;

public:
    SmartClipboard();
    ~SmartClipboard();

    // Copy to clipboard
    void copy(const std::string &text);

    // Get clipboard content
    std::string paste();

    // Get clipboard history
    std::vector<std::string> getHistory() const { return clipboard_history_; }

    // Clear history
    void clearHistory();
};

// Context-aware command builder
class CommandBuilder
{
private:
    std::string current_command_;
    std::vector<std::string> parameters_;

public:
    CommandBuilder();
    ~CommandBuilder();

    // Interactive command building
    std::string buildCommand(const std::string &base_command);

    // Show parameter suggestions
    std::vector<std::string> getParameterSuggestions(const std::string &command);

    // Validate command before execution
    bool validateCommand(const std::string &command, std::string &error_message);
};

#endif // SMART_FEATURES_H

