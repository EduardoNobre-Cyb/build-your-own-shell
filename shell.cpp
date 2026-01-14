#include <iostream>
#include <string>
#include <set>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "third_party/CLI11.hpp"
#include "include/path_utils.h"
#include "include/command_parser.h"
#include "include/command_executor.h"
#include "include/builtins.h"
#include "include/completion.h"

int main(int argc, char **argv)
{
  // Parse command line arguments with CLI11
  CLI::App app{"Custom Shell - A feature-rich command line shell"};
  
  std::string config_file;
  std::string history_file;
  bool no_history = false;
  bool verbose = false;
  
  app.add_option("-c,--config", config_file, "Configuration file path");
  app.add_option("-H,--history-file", history_file, "Custom history file path");
  app.add_flag("--no-history", no_history, "Disable command history");
  app.add_flag("-v,--verbose", verbose, "Enable verbose output");
  app.set_version_flag("-V,--version", "1.0.0");
  
  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }
  
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (verbose)
  {
    std::cout << "Starting shell in verbose mode..." << std::endl;
    if (!config_file.empty())
      std::cout << "Using config file: " << config_file << std::endl;
  }

  std::set<std::string> builtins = {"echo", "type", "exit", "pwd", "cd", "history"};

  // Track history offset for append operations
  int history_offset = 0;

  // Get history file path - priority: CLI arg > HISTFILE env > default
  std::string histfile;
  if (!history_file.empty())
  {
    histfile = history_file;
  }
  else
  {
    const char *histfile_env = getenv("HISTFILE");
    if (histfile_env)
    {
      histfile = histfile_env;
    }
    else
    {
      const char *home = getenv("HOME");
      if (home)
      {
        histfile = std::string(home) + "/.shell_history";
      }
      else
      {
        histfile = ".shell_history";
      }
    }
  }

  // Load history from file on startup (unless disabled)
  if (!no_history)
  {
    read_history(histfile.c_str());
    history_offset = history_length;
    
    if (verbose)
      std::cout << "Loaded " << history_length << " history entries from " << histfile << std::endl;
  }

  // Set up readline completion
  init_completion();
  
  if (verbose)
    std::cout << "Shell initialized. Type 'exit' or press Ctrl+D to quit." << std::endl;

  while (true)
  {
    // Build prompt with current directory
    char cwd[1024];
    std::string prompt = "$ ";
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
    {
      std::string cwd_str(cwd);
      const char *home = getenv("HOME");
      if (home && cwd_str.find(home) == 0)
      {
        // Replace home directory with ~
        cwd_str = "~" + cwd_str.substr(strlen(home));
      }
      prompt = cwd_str + " $ ";
    }

    char *input = readline(prompt.c_str());

    if (input == nullptr) // EOF (CTRL+D)
    {
      // Append new history entries before exiting (unless disabled)
      if (!no_history)
      {
        int new_entries = history_length - history_offset;
        if (new_entries > 0)
        {
          append_history(new_entries, histfile.c_str());
          if (verbose)
            std::cout << "\nSaved " << new_entries << " new history entries." << std::endl;
        }
      }
      break;
    }

    std::string command(input);

    if (!command.empty() && !no_history)
    {
      add_history(input); // Add to history for up/down arrow nav
    }

    free(input); // readline allocates memory, free it after use

    // Check for exit command
    if (command == "exit")
    {
      // Append new history entries before exiting (unless disabled)
      if (!no_history)
      {
        int new_entries = history_length - history_offset;
        if (new_entries > 0)
        {
          append_history(new_entries, histfile.c_str());
          if (verbose)
            std::cout << "Saved " << new_entries << " new history entries." << std::endl;
        }
      }
      break;
    }

    // Check if command contains a pipeline
    if (command.find('|') != std::string::npos)
    {
      // Parse pipeline and execute
      std::vector<std::string> pipeline_commands = parse_pipeline(command);
      execute_pipeline(pipeline_commands, builtins);
      continue;
    }

    // Parse for redirection
    RedirectInfo redir = parse_redirect(command);
    std::vector<std::string> args = parse_args(redir.command);

    if (args.empty())
    {
      continue;
    }

    // Handle builtin commands
    std::string cmd = args[0];

    if (cmd == "echo")
    {
      builtin_echo(args, redir.filename, redir.redirect_stderr, redir.append_mode);
    }
    else if (cmd == "pwd")
    {
      builtin_pwd();
    }
    else if (cmd == "cd")
    {
      std::string path;
      if (args.size() > 1)
      {
        path = args[1];
      }
      builtin_cd(path);
    }
    else if (cmd == "type")
    {
      if (args.size() > 1)
      {
        builtin_type(args[1], builtins);
      }
    }
    else if (cmd == "history")
    {
      builtin_history(args, history_offset);
    }
    else
    {
      // Try to execute external command
      std::string path = find_in_path(args[0]);

      if (!path.empty())
      {
        execute_command(path, args, redir.filename, redir.redirect_stderr, redir.append_mode);
      }
      else
      {
        std::cout << args[0] << ": command not found" << std::endl;
      }
    }
  } // End of while loop

  return 0;
}
