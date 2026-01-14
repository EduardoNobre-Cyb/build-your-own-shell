# Build Your Own Shell

A fully-featured Unix-like shell implementation built from scratch as part of the [CodeCrafters](https://codecrafters.io) "Build Your Own Shell" challenge.

## 🚀 Project Overview

This repository contains two complete implementations of a custom shell:

- **Python Version** ✅ Complete - Fully functional shell with all advanced features
- **C++ Version** ✅ Complete - Modular, refactored implementation with CLI11 integration

The C++ version features a modern, modular architecture with:
- Separation of concerns into multiple modules
- Command-line argument parsing using CLI11
- Professional build system with Makefile
- Comprehensive documentation

## ✨ Features

### Core Functionality

- **REPL (Read-Eval-Print Loop)** - Interactive command-line interface with dynamic directory display
- **Built-in Commands**
  - `exit` - Exit the shell
  - `echo` - Print text to stdout
  - `type` - Display command type (builtin or executable path)
  - `pwd` - Print working directory
  - `cd` - Change directory (with `~` expansion)
  - `history` - Command history management
- **External Command Execution** - Run any executable in system PATH

### Advanced Features

- **Command-Line Arguments** (C++ version)
  - `--help, -h` - Display help message
  - `--version, -V` - Show version information
  - `--verbose, -v` - Enable verbose output
  - `--no-history` - Disable command history
  - `--config, -c` - Specify configuration file
  - `--history-file, -H` - Custom history file path
- **Smart Prompt** - Shows current working directory with home directory abbreviation (`~/Documents/project $`)
- **Tab Completion**
  - Command name completion (builtins + PATH executables)
  - Directory name completion for `cd` command
  - Custom display formatting for completion matches
- **Output Redirection**
  - Standard output: `>` (overwrite), `>>` (append), `1>`, `1>>`
  - Standard error: `2>`, `2>>`
- **Multi-Command Pipelines** - Chain multiple commands with `|`
  - Supports mixing builtins and external commands
  - Proper stdin/stdout handling across process boundaries
- **Command History**
  - Display history with `history` or `history N` (last N commands)
  - Read from file: `history -r [file]`
  - Write to file: `history -w [file]` (overwrite)
  - Append to file: `history -a [file]` (append new commands only)
  - Auto-load history from `$HISTFILE` or custom file on startup
  - Auto-save new commands on exit

## 📋 Requirements

### Python Versio compiler (g++)
- POSIX-compliant system (Linux/macOS/WSL)
- GNU Readline library
- Make or mingw32-make (for building)
- CLI11 header library (included in `third_party/`)
- Standard library modules: `sys`, `os`, `subprocess`, `readline`, `shlex`

### C++ Version

- C++17 or higher
- POSIX-compliant system
- Standard library: `<iostream>`, `<string>`, `<vector>`, `<unistd.h>`, `<sys/wait.h>`

## 🔧 Installation & Usage

### Prerequisites

**For C++ Shell:**

- C++17 or higher compiler (g++ recommended)
- GNU Readline library (`libreadline-dev` on Ubuntu/Debian, `readline` on macOS)

**For Python Shell:**

- Python 3.6 or higher
- Standard library only (no additional packages needed)

### Installation

1. Clone the repository:

```bash
git clone https://github.com/EduardoNobre-Cyb/build-your-own-shell.git
cd build-your-own-shell
```

### Running the Python Shell

```bash
python3 shell.py
```

### Running the C++ Shell (Makefile, Recommended)

```bash
# Build the shell
make

# Run the shell
./bin/shell

# Or build and run
make run

# Show all options
./bin/shell --help
```

**Option 2: Using WSL on Windows**

```bash
wsl bash -c "cd /mnt/c/path/to/build-your-own-shell && make && ./bin/shell"
```

**Option 3: Legacy build script**

```bash
chmod +x build.sh
./build.sh
./bin/shell
```

**Installing Dependencies:**
Command-Line Options (C++ Version)

```bash
# Show help
$ ./bin/shell --help
Custom Shell - A feature-rich command line shell
Usage: ./bin/shell [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -V,--version                Display program version information and exit
  -v,--verbose                Enable verbose output
  --no-history                Disable command history
  -H,--history-file TEXT      Custom history file path
  -c,--config TEXT            Configuration file path

# Run with verbose mode
$ ./bin/shell --verbose

# Run without history
$ ./bin/shell --no-history

# Use custom history file
$ ./bin/shell --history-file ~/.my_shell_history
```

### 
- **Ubuntu/Debian:** `sudo apt-get install build-essential libreadline-dev`
- **Fedora/RHEL:** `sudo dnf install gcc-c++ make readline-devel`
- **macOS:** `xcode-select --install && 
- Ubuntu/Debian: `sudo apt-get install libreadline-dev`
- macOS: `brew install readline`

### Optional: History Persistence

Set the `HISTFILE` environment variable to specify where command history should be saved:

```bash
export HISTFILE=~/.my_shell_history
./shell  # or python3 shell.py
```

## 💡 Usage Examples

### Basic Commands

```bash
~/Documents $ pwd
/home/user/Documents

~/Documents $ echo Hello, World!
Hello, World!

~/Documents $ cd ~/Downloads
~/Downloads $
```

### Pipelines

- **Command Parsing**: Uses `shlex.split()` for proper quote handling
- **Pipeline Implementation**: Multi-process coordination with `subprocess.Popen`
- **Builtin Integration**: Special handling to pipe builtins through external commands
- **Tab Completion**: Custom `readline` completer with context-aware directory completion
- **History Tracking**: Global state management with write-index tracking to prevent duplicates

### C++ Implementation (Modular Architecture)

The C++ version features a clean, modular design:

```
build-your-own-shell/
├── shell.cpp                    # Main entry point with CLI11
├── Makefile                     # Cross-platform build system
├── third_party/CLI11.hpp        # CLI parsing library
├── include/                     # Header files
│   ├── path_utils.h            # PATH handling
│   ├── command_parser.h        # Argument/redirection/pipeline parsing
│ **Process Management**: `fork()`, `exec()`, `pipe()`, `wait()` system calls
- **Inter-Process Communication**: Pipelines, file descriptors, stream redirection
- **String Parsing**: Quote handling, tokenization, command interpretation
- **Terminal Programming**: GNU readline integration, tab completion, history
- **System Programming**: PATH resolution, environment variables, signal handling
- **Software Architecture**: Modular design, separation of concerns, clean code
- **Modern C++**: C++17 features, STL containers, RAII patterns
- **Build Systems**: Makefile, cross-platform compilation
- **Command-Line Interfaces**: CLI11 library, user-friendly option parsing
- **Error Handling**: Edge cases, resource management, graceful failures

## 📚 Documentation

Additional documentation is available in the [docs/](docs/) folder:

- **[CLI_LIBRARIES.md](docs/CLI_LIBRARIES.md)** - Comparison of C++ CLI parsing libraries
- **[REFACTORING_SUMMARY.md](docs/REFACTORING_SUMMARY.md)** - Detailed refactoring overview
- **[BUILD.md](docs/BUILD.md)** - Comprehensive build instructions
- **[QUICK_REFERENCE.md](docs/QUICK_REFERENCE.md)** - Quick command reference
- **[CLI11_INTEGRATION_COMPLETE.md](docs/CLI11_INTEGRATION_COMPLETE.md)** - CLI11 integration details
    ├── command_executor.cpp
    ├── builtins.cpp
    └── completion.cpp
```

**Key Design Principles:**
- **Separation of Concerns**: Each module has a single, clear responsibility
- **Modular Design**: Easy to test, maintain, and extend
- **Modern C++**: C++17 features with clean, readable code
- **CLI11 Integration**: Professional command-line argument parsing
- **Cross-Platform**: Works on Linux, macOS, and Windows (via WSL)

**Modules:**
- **path_utils**: Executable lookup and PATH resolu

**C++ Version**: Modular architecture with CLI11 integration 🚀tion
- **command_parser**: Parse commands, handle quotes, redirections, and pipes
- **command_executor**: Execute commands with process management
- **builtins**: Implement shell builtin commands
- **completion**: Readline tab completion integration

**Build System:**
```bash
make              # Build main shell
make both         # Build both versions
make original     # Build original monolithic version
make clean        # Remove build artifacts
make help         # Show all targets
```
$ command_with_errors 2> errors.log
```

### History Management

```bash
$ history          # Show all commands
   1  pwd
   2  echo test
   3  history

$ history 5        # Show last 5 commands

$ history -w ~/.shell_history    # Save history to file
$ history -a ~/.shell_history    # Append new commands to file
$ history -r ~/.shell_history    # Load history from file
```

### Tab Completion

```bash
$ ec<TAB>          # Completes to "echo "
$ cd Doc<TAB>      # Completes to "cd Documents/"
```

## 🏗️ Architecture

### Python Implementation Highlights

- **Command Parsing**: Uses `shlex.split()` for proper quote handling
- **Pipeline Implementation**: Multi-process coordination with `subprocess.Popen`
- **Builtin Integration**: Special handling to pipe builtins through external commands
- **Tab Completion**: Custom `readline` completer with context-aware directory completion
- **History Tracking**: Global state management with write-index tracking to prevent duplicates

### C++ Implementation Highlights

- **Command Parsing**: Custom tokenizer with quote handling
- **Pipeline Implementation**: Process coordination using `fork()`, `pipe()`, and `execvp()`
- **Builtin Integration**: Direct implementation with proper stream redirection
- **Tab Completion**: GNU readline library integration for command and directory completion
- **History Tracking**: File-based persistence with duplicate prevention

## 🎯 Learning Outcomes

This project demonstrates proficiency in:

- Process management and inter-process communication
- File I/O and stream redirection
- String parsing and command interpretation
- Terminal interaction and readline integration
- System programming concepts (PATH resolution, environment variables)
- Error handling and edge case management

## 🙏 Acknowledgments

This project is part of the [CodeCrafters](https://codecrafters.io) "Build Your Own Shell" challenge, which provides a structured learning path for building complex systems from scratch.

## 📝 License

This project is open source and available under the [MIT License](LICENSE).

---

**Status**: Both Python and C++ versions complete ✅
