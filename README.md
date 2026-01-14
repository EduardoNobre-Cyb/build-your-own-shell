# Build Your Own Shell

A fully-featured Unix-like shell implementation built from scratch as part of the [CodeCrafters](https://codecrafters.io) "Build Your Own Shell" challenge.

## 🚀 Project Overview

This repository contains two complete implementations of a custom shell, both built from the same CodeCrafters challenge:

- **Python Version** ✅ Complete - Fully functional shell with all advanced features
- **C++ Version** ✅ Complete - Fully functional shell with all advanced features

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
  - Auto-load history from `$HISTFILE` on startup
  - Auto-save new commands to `$HISTFILE` on exit

## 📋 Requirements

### Python Version

- Python 3.6+
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

### Running the C++ Shell

**Option 1: Using the build script**

```bash
chmod +x build.sh
./build.sh
./shell
```

**Option 2: Manual compilation**

```bash
g++ -std=c++17 -o shell shell.cpp -lreadline
./shell
```

**Note:** On some systems you may need to install readline:

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

```bash
$ echo "apple\nbanana\ncherry" | grep apple
apple

$ cat file.txt | grep error | wc -l
5
```

### Output Redirection

```bash
$ echo "Log entry" > output.txt
$ echo "Another entry" >> output.txt
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
