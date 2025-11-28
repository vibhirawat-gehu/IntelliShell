#include "autocomplete.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstring>

AutocompleteEngine::AutocompleteEngine()
{
    initializeCommandList();
}

AutocompleteEngine::~AutocompleteEngine()
{
    // Cleanup
}

void AutocompleteEngine::initializeCommandList()
{
    // Built-in commands
    command_list_ = {
        "help", "exit", "cd", "ls", "dir", "pwd", "cat", "type",
        "echo", "clear", "cls", "whoami", "date", "time",
        "alias", "unalias", "history", "suggest", "resources",
        "users", "roles", "permissions", "login", "logout", "sudo"};

    // Common shortcuts
    command_list_.push_back("ll");
    command_list_.push_back("la");
    command_list_.push_back("ps");
    command_list_.push_back("net");
    command_list_.push_back("findf");
}

std::vector<CompletionOption> AutocompleteEngine::getCompletions(const std::string &partial)
{
    std::vector<CompletionOption> completions;

    if (partial.empty())
    {
        // Return all commands if empty
        for (const auto &cmd : command_list_)
        {
            CompletionOption opt;
            opt.text = cmd;
            opt.description = "Built-in command";
            opt.match_score = 100;
            completions.push_back(opt);
        }
        return completions;
    }

    // Find matching commands
    for (const auto &cmd : command_list_)
    {
        if (cmd.find(partial) == 0)
        { // Starts with partial
            CompletionOption opt;
            opt.text = cmd;
            opt.description = "Command";
            opt.match_score = calculateMatchScore(partial, cmd);
            completions.push_back(opt);
        }
    }

    // Search in recent commands
    for (const auto &recent : recent_commands_)
    {
        if (recent.find(partial) == 0 &&
            std::find_if(completions.begin(), completions.end(),
                         [&recent](const CompletionOption &o)
                         { return o.text == recent; }) == completions.end())
        {
            CompletionOption opt;
            opt.text = recent;
            opt.description = "Recent command";
            opt.match_score = calculateMatchScore(partial, recent) + 50; // Boost recent commands
            completions.push_back(opt);
        }
    }

    // Sort by match score
    std::sort(completions.begin(), completions.end(),
              [](const CompletionOption &a, const CompletionOption &b)
              {
                  return a.match_score > b.match_score;
              });

    // Limit to top 10
    if (completions.size() > 10)
    {
        completions.resize(10);
    }

    return completions;
}

std::string AutocompleteEngine::completeCommand(const std::string &partial)
{
    auto completions = getCompletions(partial);
    if (!completions.empty())
    {
        return completions[0].text;
    }
    return partial;
}

void AutocompleteEngine::displayNavigationBar(const std::vector<CompletionOption> &options, int selected)
{
    if (options.empty())
        return;

    // Save cursor position
    std::cout << "\033[s"; // Save

    // Move cursor down 1 line from current position
    std::cout << "\n";

    // Clear line
    std::cout << "\033[K"; // Clear to end of line

    // Show suggestions inline like VS Code
    std::cout << "\033[90m  Suggestions: \033[0m";

    int max_show = 5; // Max commands to show
    int start = (selected >= max_show) ? selected - max_show + 1 : 0;
    int end = std::min(start + max_show, static_cast<int>(options.size()));

    for (int i = start; i < end; i++)
    {
        bool is_selected = (i == selected);
        if (is_selected)
        {
            // Highlight selected with background color
            std::cout << "\033[7m\033[1;36m " << options[i].text << " \033[0m";
        }
        else
        {
            std::cout << "\033[90m " << options[i].text << " \033[0m";
        }
        if (i < end - 1)
            std::cout << " ";
    }

    if (options.size() > max_show)
    {
        std::cout << " \033[90m(" << (selected + 1) << "/" << options.size() << ")\033[0m";
    }

    std::cout << " \033[90m[Tab=accept, Arrows=navigate, Esc=cancel]\033[0m";
    std::cout.flush();

    // Restore cursor position
    std::cout << "\033[u"; // Restore
}

void AutocompleteEngine::clearNavigationBar()
{
    // Save cursor position
    std::cout << "\033[s";

    // Move down 1 line and clear it
    std::cout << "\n\033[K";
    std::cout << "\033[1A"; // Move up to original position

    // Restore cursor position
    std::cout << "\033[u";
    std::cout.flush();
}

void AutocompleteEngine::addToHistory(const std::string &command)
{
    // Add to recent commands (keep last 20)
    recent_commands_.push_back(command);
    if (recent_commands_.size() > 20)
    {
        recent_commands_.erase(recent_commands_.begin());
    }
}

std::vector<std::string> AutocompleteEngine::getRelatedCommands(const std::string &base_command)
{
    std::vector<std::string> related;

    // Find similar commands (simple prefix matching)
    for (const auto &cmd : command_list_)
    {
        if (cmd.find(base_command) == 0 || base_command.find(cmd) == 0)
        {
            if (cmd != base_command)
            {
                related.push_back(cmd);
            }
        }
    }

    return related;
}

std::string AutocompleteEngine::formatCompletion(const CompletionOption &option, bool selected)
{
    if (selected)
    {
        return "\033[7m\033[1m" + option.text + "\033[0m"; // Reverse video, bold
    }
    else
    {
        return "\033[90m" + option.text + "\033[0m"; // Gray
    }
}

int AutocompleteEngine::calculateMatchScore(const std::string &partial, const std::string &candidate) const
{
    if (partial == candidate)
        return 100;
    if (candidate.find(partial) == 0)
        return 80; // Starts with
    if (candidate.find(partial) != std::string::npos)
        return 50; // Contains
    return 0;
}


