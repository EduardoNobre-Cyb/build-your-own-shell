# CLI11 Integration

The shell now uses **CLI11** for command-line argument parsing, providing a modern and user-friendly interface for shell configuration.

## Integrated Features

### Command-Line Options

| Option | Short | Description | Example |
|--------|-------|-------------|---------|
| `--help` | `-h` | Display help message | `./shell -h` |
| `--version` | `-V` | Show version information | `./shell --version` |
| `--config` | `-c` | Specify configuration file | `./shell -c myconfig.conf` |
| `--history-file` | `-H` | Custom history file path | `./shell -H ~/.my_history` |
| `--no-history` | | Disable command history | `./shell --no-history` |
| `--verbose` | `-v` | Enable verbose output | `./shell -v` |

## Usage Examples

### Basic Usage
```bash
# Run shell normally
./bin/shell_refactored

# Show help
./bin/shell_refactored --help

# Show version
./bin/shell_refactored --version
```

### Custom Configuration
```bash
# Use custom config file
./bin/shell_refactored --config ~/.myshell.conf

# Use custom history file
./bin/shell_refactored --history-file ~/.custom_history

# Disable history completely
./bin/shell_refactored --no-history
```

### Verbose Mode
```bash
# Run with verbose output to see what's happening
./bin/shell_refactored --verbose

# Example output:
# Starting shell in verbose mode...
# Loaded 42 history entries from /home/user/.shell_history
# Shell initialized. Type 'exit' or press Ctrl+D to quit.
```

### Combined Options
```bash
# Multiple options can be combined
./bin/shell_refactored -v -c config.txt -H my_hist.txt

# Short form
./bin/shell_refactored -vH custom_history.txt
```

## Implementation Details

### CLI11 Setup
- **Library Version:** v2.4.1
- **Type:** Header-only (single file: `CLI11.hpp`)
- **Location:** `third_party/CLI11.hpp`
- **C++ Standard:** C++11 or later (using C++17 in this project)

### Argument Parsing
The main function signature was updated from:
```cpp
int main()
```

To:
```cpp
int main(int argc, char **argv)
```

### Configuration Priority
History file path resolution follows this priority order:
1. **Command-line argument** (`--history-file`)
2. **Environment variable** (`$HISTFILE`)
3. **Default location** (`~/.shell_history`)

### Code Changes
Key modifications to `shell_refactored.cpp`:

1. **CLI11 Header Included:**
   ```cpp
   #include "third_party/CLI11.hpp"
   ```

2. **Argument Parser Setup:**
   ```cpp
   CLI::App app{"Custom Shell - A feature-rich command line shell"};
   app.add_option("-c,--config", config_file, "Configuration file path");
   app.add_flag("--no-history", no_history, "Disable command history");
   // ... more options
   ```

3. **Parse and Handle Errors:**
   ```cpp
   try {
       app.parse(argc, argv);
   } catch (const CLI::ParseError &e) {
       return app.exit(e);
   }
   ```

## Benefits

### User Experience
- ✅ **Intuitive:** Familiar command-line interface patterns
- ✅ **Self-documenting:** Built-in `--help` shows all options
- ✅ **Flexible:** Easy to customize shell behavior
- ✅ **Discoverable:** Users can explore options with `--help`

### Developer Experience
- ✅ **Type-safe:** Compile-time type checking for options
- ✅ **Simple API:** Clean, readable argument definitions
- ✅ **Automatic help:** Help text generated automatically
- ✅ **Error handling:** Clear error messages for invalid input
- ✅ **Validation:** Built-in validators for common cases

### Extensibility
Adding new options is straightforward:

```cpp
// Add a new boolean flag
bool debug = false;
app.add_flag("-d,--debug", debug, "Enable debug mode");

// Add a new option with validation
int max_history = 1000;
app.add_option("--max-history", max_history, "Maximum history entries")
    ->check(CLI::Range(1, 10000));

// Add a new option with default value
std::string theme = "default";
app.add_option("-t,--theme", theme, "Color theme")
    ->default_str("default");
```

## Testing

### Test Help Output
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

### Test Version
```bash
./bin/shell_refactored --version
```

Expected output:
```
1.0.0
```

### Test Invalid Options
```bash
./bin/shell_refactored --invalid-option
```

Expected output:
```
--invalid-option is not a recognised option
Run with --help for more information.
```

## Future Enhancements

Potential additional options to implement:

1. **Color scheme:** `--color=always|auto|never`
2. **Prompt customization:** `--prompt="$ "`
3. **Script execution:** `--script=file.sh`
4. **Non-interactive mode:** `--non-interactive`
5. **Logging:** `--log-file=shell.log`
6. **Resource limits:** `--max-jobs=10`
7. **Plugin directory:** `--plugin-dir=/path/to/plugins`

## Comparison: Before vs After

### Before (No CLI11)
```bash
# Limited: Only environment variables
export HISTFILE=~/.my_history
./shell

# No help available
# No version info
# No way to disable features
```

### After (With CLI11)
```bash
# Flexible: Command-line options
./shell --history-file ~/.my_history --verbose

# Built-in help
./shell --help

# Version information
./shell --version

# Feature control
./shell --no-history
```

## References

- **CLI11 Documentation:** https://cliutils.github.io/CLI11/book/
- **CLI11 GitHub:** https://github.com/CLIUtils/CLI11
- **CLI11 Tutorial:** https://cliutils.github.io/CLI11/book/chapters/basics.html
