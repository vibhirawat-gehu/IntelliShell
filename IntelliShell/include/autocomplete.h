#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <string>
#include <vector>
#include <functional>

struct CompletionOption {
    std::string text;
    std::string description;
    int match_score;
};

class AutocompleteEngine {
private:
    std::vector<std::string> command_list_;
    std::vector<std::string> recent_commands_;
    
    void initializeCommandList();
    int calculateMatchScore(const std::string& partial, const std::string& candidate) const;
    
public:
    AutocompleteEngine();
    ~AutocompleteEngine();
    
    // Tab completion
    std::vector<CompletionOption> getCompletions(const std::string& partial);
    std::string completeCommand(const std::string& partial);
    
    // Navigation bar
    void displayNavigationBar(const std::vector<CompletionOption>& options, int selected = 0);
    void clearNavigationBar();
    
    // Update history
    void addToHistory(const std::string& command);
    
    // Get suggestions
    std::vector<std::string> getRelatedCommands(const std::string& base_command);
    
    // Utility
    std::string formatCompletion(const CompletionOption& option, bool selected = false);
};

#endif // AUTOCOMPLETE_H




