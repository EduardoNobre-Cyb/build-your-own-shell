# C++ Command Line Argument Parsing Libraries

This document provides an overview of popular, actively maintained, multiplatform C++ command line parsing libraries.

## 1. CLI11 ⭐ **RECOMMENDED**

**Repository:** https://github.com/CLIUtils/CLI11

**Pros:**
- Modern C++11/14/17 header-only library
- Very intuitive and easy to use API
- Excellent documentation with many examples
- Active development and maintenance
- Multiplatform (Windows, Linux, macOS)
- Supports subcommands, option groups, validators
- No external dependencies
- Great for both simple and complex CLIs

**Installation:**
```bash
# Header-only - just copy CLI11.hpp or use package managers:
# vcpkg: vcpkg install cli11
# conan: cli11/2.3.2
```

**Example Usage:**
```cpp
#include "CLI/CLI.hpp"

int main(int argc, char** argv) {
    CLI::App app{"Shell Application"};
    
    std::string config_file = "default.conf";
    bool verbose = false;
    int port = 8080;
    
    app.add_option("-c,--config", config_file, "Configuration file");
    app.add_flag("-v,--verbose", verbose, "Enable verbose output");
    app.add_option("-p,--port", port, "Port number")->check(CLI::Range(1, 65535));
    
    CLI11_PARSE(app, argc, argv);
    
    return 0;
}
```

---

## 2. cxxopts

**Repository:** https://github.com/jarro2783/cxxopts

**Pros:**
- Lightweight header-only library
- Modern C++11 or later
- Getopt-like syntax (familiar to Unix users)
- Multiplatform
- Good documentation
- Active maintenance

**Installation:**
```bash
# Header-only - include cxxopts.hpp
# vcpkg: vcpkg install cxxopts
# conan: cxxopts/3.1.1
```

**Example Usage:**
```cpp
#include "cxxopts.hpp"

int main(int argc, char** argv) {
    cxxopts::Options options("shell", "A shell application");
    
    options.add_options()
        ("c,config", "Config file", cxxopts::value<std::string>())
        ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
        ("p,port", "Port number", cxxopts::value<int>()->default_value("8080"))
        ("h,help", "Print help");
    
    auto result = options.parse(argc, argv);
    
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    
    return 0;
}
```

---

## 3. argparse (Python-inspired)

**Repository:** https://github.com/p-ranav/argparse

**Pros:**
- Python argparse-inspired API (great for Python developers)
- Modern C++17 single-header library
- Multiplatform
- Automatic help generation
- Type-safe
- Active development

**Installation:**
```bash
# Single header library
# vcpkg: vcpkg install argparse
# conan: argparse/2.9
```

**Example Usage:**
```cpp
#include <argparse/argparse.hpp>

int main(int argc, char** argv) {
    argparse::ArgumentParser program("shell");
    
    program.add_argument("-c", "--config")
        .default_value(std::string{"default.conf"})
        .help("configuration file");
    
    program.add_argument("-v", "--verbose")
        .default_value(false)
        .implicit_value(true)
        .help("enable verbose output");
    
    program.add_argument("-p", "--port")
        .default_value(8080)
        .scan<'i', int>()
        .help("port number");
    
    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }
    
    return 0;
}
```

---

## 4. Boost.Program_options

**Repository:** https://www.boost.org/doc/libs/release/libs/program_options/

**Pros:**
- Part of the well-established Boost library ecosystem
- Very mature and stable
- Extensive features
- Multiplatform
- Well-documented
- Supports config files and command line parsing

**Cons:**
- Requires Boost dependency (large library)
- More verbose API
- Not header-only (requires linking)

**Installation:**
```bash
# Requires Boost installation
# vcpkg: vcpkg install boost-program-options
# apt: sudo apt-get install libboost-program-options-dev
```

**Example Usage:**
```cpp
#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main(int argc, char** argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("config,c", po::value<std::string>()->default_value("default.conf"), "config file")
        ("verbose,v", "verbose output")
        ("port,p", po::value<int>()->default_value(8080), "port number");
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    
    return 0;
}
```

---

## Comparison Summary

| Library | C++ Standard | Header-Only | Dependencies | Ease of Use | Active |
|---------|--------------|-------------|--------------|-------------|--------|
| **CLI11** | C++11+ | ✅ Yes | None | ⭐⭐⭐⭐⭐ | ✅ |
| **cxxopts** | C++11+ | ✅ Yes | None | ⭐⭐⭐⭐ | ✅ |
| **argparse** | C++17 | ✅ Yes | None | ⭐⭐⭐⭐⭐ | ✅ |
| **Boost.Program_options** | C++03+ | ❌ No | Boost | ⭐⭐⭐ | ✅ |

## Recommendation for This Project

For the shell project, **CLI11** is the best choice because:
1. **Easy integration**: Header-only, no dependencies
2. **Modern**: Clean C++11+ API
3. **Flexible**: Simple for basic needs, powerful for advanced use
4. **Active**: Well-maintained with excellent documentation
5. **Multiplatform**: Works seamlessly on Windows, Linux, and macOS

**Alternative**: If you prefer Python-like syntax, **argparse** is also excellent (requires C++17).
