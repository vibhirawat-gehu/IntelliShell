# Smart Shell - Intelligent Command Interface

A smart, resource-aware shell that combines traditional shell functionality with AI-powered suggestions, resource monitoring, and enhanced usability features.

## Features

### Core Features
- **Smart Command Suggestions**: Context-aware command recommendations based on usage patterns
- **Resource Monitoring**: Real-time CPU, memory, and process monitoring with warnings
- **Auto-aliasing**: Automatic creation of aliases for frequently used commands
- **Command History**: Enhanced history with search and navigation
- **Background Process Execution**: Support for background processes with `&`
- **I/O Redirection**: Full support for `<`, `>`, and `>>` redirection

### Built-in Commands
- `cd [directory]` - Change directory
- `alias [name=cmd]` - Create or list aliases
- `unalias <name>` - Remove alias
- `history [count]` - Show command history
- `help` - Show help information
- `exit` - Exit shell
- `resources` - Show system resource usage
- `suggest <partial>` - Get command suggestions
- `users` - List system users
- `roles` - List system roles
- `permissions` - List available permissions
- `login <username>` - Login as user
- `logout` - Logout current user
- `sudo <command>` - Execute command with admin privileges

### Shortcuts
- `ll` / `la` - `ls -la` (detailed listing)
- `ps` - `ps aux` (process list)
- `net` - `netstat -tuln` (network status)
- `findf <pattern>` - `find . -name <pattern>`

### Advanced Features
- **Resource Warnings**: Automatic warnings when CPU/memory usage is high
- **Smart Corrections**: Automatic correction of common typos
- **Context-aware Suggestions**: Suggestions based on current directory and system state
- **Multi-session Support**: Background process management
- **Enhanced UI**: Optional ncurses-based interface with real-time resource display
- **Role-Based Access Control (RBAC)**: User authentication and permission system
- **Security Features**: Command-level permission checking and user management

## Building

### Prerequisites
- Linux environment (Ubuntu/Debian recommended)
- G++ compiler (C++17 support)
- ncurses development libraries
- POSIX-compliant system

### Installation
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y build-essential libncurses5-dev

# Build the project
make

# Run the shell
./smart_shell
```

### Build Options
```bash
# Debug build
make debug

# Clean build artifacts
make clean

# Install dependencies
make install-deps
```

## Usage

### Basic Usage
```bash
# Start the shell
./smart_shell

# Use built-in commands
$ help
$ resources
$ suggest ls

# Use shortcuts
$ ll
$ ps
$ net

# Create aliases
$ alias ll=ls -la
$ alias myfind=find . -name

# Background processes
$ long_running_command &

# I/O redirection
$ ls > output.txt
$ grep pattern < input.txt
$ echo "append" >> log.txt
```

### Advanced Features
```bash
# Get suggestions for partial commands
$ suggest find

# Monitor system resources
$ resources

# View command history
$ history 20

# RBAC commands
$ users
$ roles
$ permissions
$ login admin
$ sudo system_command
$ logout

# Auto-aliasing (automatic for frequently used commands)
# Commands used >5 times get auto-aliased
```

## Architecture

### System Components
1. **Command Parser** - Interprets user input and handles redirection
2. **Execution Engine** - Manages process creation and I/O handling
3. **Suggestion Engine** - Provides intelligent command recommendations
4. **Resource Monitor** - Tracks system resource usage
5. **File/Network Module** - Shortcuts for common operations
6. **UI Interface** - Optional ncurses-based interface
7. **Security Layer** - Role-based access control (optional)

### File Structure
```
src/
├── main.cpp              # Main entry point
├── shell_core.cpp        # Core shell functionality (OOP)
├── builtin_commands.cpp  # Built-in command implementations
├── suggestion_engine.cpp # AI-powered suggestions
├── resource_monitor.cpp  # System resource monitoring (OOP)
├── shortcuts.cpp         # File/network shortcuts
├── ui_interface.cpp      # ncurses UI implementation (OOP)
└── rbac_system.cpp      # RBAC system (OOP)

include/
└── smart_shell.h         # C++ header with classes and modern features

obj/                    # Object files (generated)
smart_shell            # Executable (generated)
```

## Configuration

### Alias File
The shell automatically saves and loads aliases from `.smart_shell_aliases` in the current directory.

### Resource Monitoring
- CPU usage threshold: 80%
- Memory usage threshold: 1GB
- Process count threshold: 500
- Check interval: 2 seconds

## Development

### Adding New Built-in Commands
1. Add command name to `is_builtin_command()` in `shell_core.c`
2. Add handler to `execute_builtin()` in `shell_core.c`
3. Implement the command function in `builtin_commands.c`
4. Add help text to `cmd_help()` in `builtin_commands.c`

### Adding New Shortcuts
1. Add shortcut name to `is_shortcut_command()` in `shortcuts.c`
2. Add handler to `execute_shortcut()` in `shortcuts.c`
3. Implement the shortcut function in `shortcuts.c`

### Extending Suggestions
1. Add new suggestion patterns to `find_common_suggestions()` in `suggestion_engine.c`
2. Implement context-aware suggestions based on system state
3. Add machine learning capabilities for better suggestions

## Performance

### Benchmarks
- Command parsing: <1ms
- Resource monitoring: <10ms
- Suggestion generation: <5ms
- Memory usage: ~2MB base + command history

### Optimization
- Efficient string operations
- Minimal system calls
- Lazy resource monitoring
- Cached suggestions

## Security

### Current Security Features
- Input validation
- Safe command execution
- Resource usage limits

### Planned Security Features
- Role-based access control (RBAC)
- Command whitelisting
- Process sandboxing
- Audit logging

## Troubleshooting

### Common Issues
1. **ncurses not found**: Install `libncurses5-dev`
2. **Permission denied**: Check file permissions
3. **Resource warnings**: Normal behavior when system is under load
4. **Suggestions not working**: Check command patterns in suggestion engine

### Debug Mode
```bash
make debug
./smart_shell
# Debug output will show detailed execution information
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is part of an Operating Systems PBL (Project-Based Learning) course.

## References

- Advanced Programming in the UNIX Environment – W. Richard Stevens
- Linux Manual Pages (man bash, man exec, man fork)
- Fish Shell Documentation: https://fishshell.com/
- POSIX Standards: https://pubs.opengroup.org/onlinepubs/9699919799/

## Project Timeline

- **Week 1–2**: Literature review, architecture & technology stack ✅
- **Week 3–4**: Core shell functionalities ✅
- **Week 5–6**: Smart command suggestions & auto-aliasing ✅
- **Week 7**: File/network management shortcuts ✅
- **Week 8**: Resource-awareness features ✅
- **Week 9**: Multi-session handling & RBAC (optional)
- **Week 10**: Testing, debugging, and documentation ✅
