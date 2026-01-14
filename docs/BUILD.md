# Build Instructions

## Prerequisites

The shell requires:
- **C++ Compiler:** g++ with C++17 support
- **readline library:** For line editing and history
- **make:** For building with the Makefile

## Building on Windows

### Option 1: WSL (Windows Subsystem for Linux) - Recommended
```bash
# In PowerShell or Windows Terminal
wsl bash -c "cd /mnt/c/dev/src/dudu/build-your-own-shell && make refactored"

# To run:
wsl bash -c "cd /mnt/c/dev/src/dudu/build-your-own-shell && ./bin/shell_refactored"
```

### Option 2: MinGW/MSYS2
```bash
# Install MSYS2 from https://www.msys2.org/
# In MSYS2 terminal:
pacman -S mingw-w64-x86_64-gcc make mingw-w64-x86_64-readline

# Navigate to project directory
cd /c/dev/src/dudu/build-your-own-shell

# Build
make refactored
```

### Option 3: Visual Studio with WSL Extension
- Open the project folder in VS Code
- Use WSL extension to build and run

## Building on Linux/macOS

### Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential libreadline-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc-c++ make readline-devel
```

**macOS:**
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install readline via Homebrew (optional, macOS has built-in version)
brew install readline
```

### Build Commands

```bash
# Clone or navigate to the project directory
cd build-your-own-shell

# Build refactored version
make refactored

# Or build both versions
make all

# Clean build artifacts
make clean

# Rebuild everything
make rebuild
```

## Testing the Build

### Check Help
```bash
./bin/shell_refactored --help
```

Expected output:
```
Custom Shell - A feature-rich command line shell
Usage: ./bin/shell_refactored [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -V,--version                Display program version information and exit
  -c,--config TEXT            Configuration file path
  -H,--history-file TEXT      Custom history file path
  --no-history                Disable command history
  -v,--verbose                Enable verbose output
```

### Check Version
```bash
./bin/shell_refactored --version
```

Expected output:
```
1.0.0
```

### Run the Shell
```bash
# Run normally
./bin/shell_refactored

# Run with verbose mode
./bin/shell_refactored --verbose

# Run without history
./bin/shell_refactored --no-history
```

## Troubleshooting

### Error: "readline/readline.h: No such file or directory"
**Solution:** Install readline development library
```bash
# Ubuntu/Debian
sudo apt-get install libreadline-dev

# Fedora/RHEL
sudo dnf install readline-devel

# macOS
brew install readline
```

### Error: "CLI11.hpp: No such file or directory"
**Solution:** Ensure CLI11 header is downloaded
```bash
curl -L https://github.com/CLIUtils/CLI11/releases/download/v2.4.1/CLI11.hpp -o third_party/CLI11.hpp
```

### Make command not found
**Solution:** Install make
```bash
# Ubuntu/Debian
sudo apt-get install make

# macOS
xcode-select --install

# Windows: Use WSL or MSYS2
```

### Permission denied when running
**Solution:** Make the binary executable
```bash
chmod +x bin/shell_refactored
```

## Build Targets

| Target | Description |
|--------|-------------|
| `make all` | Build both original and refactored versions |
| `make refactored` | Build only refactored version with CLI11 |
| `make original` | Build only original version |
| `make clean` | Remove all build artifacts |
| `make rebuild` | Clean and rebuild |
| `make run-refactored` | Build and run refactored version |
| `make help` | Show all available targets |

## Project Structure After Build

```
build-your-own-shell/
├── bin/
│   ├── shell                  # Original version executable
│   └── shell_refactored       # Refactored version with CLI11
├── build/
│   ├── shell.o               # Original object file
│   ├── shell_refactored.o    # Main object file
│   └── src/                  # Module object files
│       ├── path_utils.o
│       ├── command_parser.o
│       ├── command_executor.o
│       ├── builtins.o
│       └── completion.o
└── ... (source files)
```

## Development Workflow

### Incremental Builds
The Makefile supports incremental compilation - only modified files are recompiled:

```bash
# Edit a single module, e.g., src/builtins.cpp
vim src/builtins.cpp

# Only builtins.cpp and final linking will be executed
make refactored
```

### Debugging Build
```bash
# Add debug symbols
make CXXFLAGS="-std=c++17 -Wall -Wextra -g -O0" refactored

# Run with gdb
gdb ./bin/shell_refactored
```

### Clean Build
```bash
# Remove all artifacts and rebuild
make rebuild
```

## CI/CD Integration

### GitHub Actions Example
```yaml
name: Build Shell

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt-get install -y libreadline-dev
      - name: Download CLI11
        run: |
          mkdir -p third_party
          curl -L https://github.com/CLIUtils/CLI11/releases/download/v2.4.1/CLI11.hpp -o third_party/CLI11.hpp
      - name: Build
        run: make refactored
      - name: Test
        run: ./bin/shell_refactored --version
```

## Performance

Typical build times on modern hardware:
- **Clean build:** 3-5 seconds
- **Incremental (1 file change):** < 1 second
- **Full rebuild:** 3-5 seconds

Build artifact sizes:
- **shell_refactored:** ~200-300 KB (stripped)
- **Total build directory:** ~500 KB
