#include "smart_shell.h"
#ifdef _WIN32
    #include <io.h>
#endif

int main(int argc, char *argv[]) {
    (void)argc; (void)argv; // Suppress unused parameter warnings
    ShellState shell;
    
    // Initialize shell state
    if (!shell.initialize()) {
        printError("Failed to initialize shell");
        return 1;
    }
    
    // Check if running in interactive mode
#ifdef _WIN32
    // Windows: check if stdin is a console
    DWORD mode;
    shell.setInteractive(GetFileType(GetStdHandle(STD_INPUT_HANDLE)) == FILE_TYPE_CHAR);
#else
    shell.setInteractive(isatty(STDIN_FILENO));
#endif
    
    // Print welcome message
    if (shell.isInteractive()) {
        std::cout << "Smart Shell v2.0 - Intelligent Command Interface (C++)" << std::endl;
        std::cout << "Type 'help' for available commands, 'exit' to quit" << std::endl;
        std::cout << "Features: Smart suggestions, resource monitoring, shortcuts\n" << std::endl;
    }
    
    // Main shell loop
    int result = shell.run();
    
    // Cleanup
    shell.cleanup();
    
    return result;
}


