#include "include/builtins.h"
#include "include/path_utils.h"
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <readline/history.h>

void builtin_echo(const std::vector<std::string> &args,
                  const std::string &redirect_file,
                  bool redirect_stderr,
                  bool append_mode)
{
  int saved_fd = -1;
  if (!redirect_file.empty())
  {
    // Save original fd (stdout or stderr)
    int fd_to_redirect = redirect_stderr ? STDERR_FILENO : STDOUT_FILENO;
    saved_fd = dup(fd_to_redirect);

    // Choose flags based on append mode
    int flags = O_WRONLY | O_CREAT;
    if (append_mode)
    {
      flags |= O_APPEND; // Append to end of file
    }
    else
    {
      flags |= O_TRUNC; // Truncate (overwrite) file
    }

    // Redirect to file
    int fd = open(redirect_file.c_str(), flags, 0644);
    if (fd < 0)
    {
      std::cerr << "Error: cannot open file for writing" << std::endl;
      return;
    }
    dup2(fd, fd_to_redirect);
    close(fd);
  }

  // Print all arguments except the first one (which is "echo")
  for (size_t i = 1; i < args.size(); i++)
  {
    if (i > 1)
      std::cout << " "; // Add space between arguments
    std::cout << args[i];
  }
  std::cout << std::endl;

  if (!redirect_file.empty())
  {
    // Restore original fd
    int fd_to_redirect = redirect_stderr ? STDERR_FILENO : STDOUT_FILENO;
    dup2(saved_fd, fd_to_redirect);
    close(saved_fd);
  }
}

void builtin_pwd()
{
  char cwd[1024]; // Buffer for current working directory
  if (getcwd(cwd, sizeof(cwd)) == nullptr)
  {
    std::cerr << "pwd: error retrieving current directory" << std::endl;
  }
  else
  {
    std::cout << cwd << std::endl;
  }
}

bool builtin_cd(const std::string &path)
{
  std::string target_path = path;

  if (target_path.empty())
  {
    // No argument - go to HOME
    const char *home = std::getenv("HOME");
    if (home != nullptr)
    {
      target_path = home;
    }
    else
    {
      std::cerr << "cd: HOME not set" << std::endl;
      return false;
    }
  }
  else
  {
    // Handle tilde expansion
    if (target_path == "~" || target_path.substr(0, 2) == "~/")
    {
      const char *home = std::getenv("HOME");
      if (home != nullptr)
      {
        if (target_path == "~")
        {
          target_path = home;
        }
        else
        {
          target_path = std::string(home) + target_path.substr(1); // Replace ~ with HOME
        }
      }
    }
  }

  // Change directory
  if (chdir(target_path.c_str()) != 0)
  {
    std::cerr << "cd: " << target_path << ": No such file or directory" << std::endl;
    return false;
  }
  return true;
}

void builtin_type(const std::string &arg, const std::set<std::string> &builtins)
{
  if (builtins.count(arg) > 0)
  {
    std::cout << arg << " is a shell builtin" << std::endl;
  }
  else
  {
    std::string path = find_in_path(arg);
    if (!path.empty())
    {
      std::cout << arg << " is " << path << std::endl;
    }
    else
    {
      std::cout << arg << ": not found" << std::endl;
    }
  }
}

bool builtin_history(const std::vector<std::string> &args, int &history_offset)
{
  // Case 1: Just "history" - show all
  if (args.size() == 1)
  {
    HIST_ENTRY **hist_list = history_list();
    if (hist_list == nullptr || history_length == 0)
    {
      return true;
    }

    for (int i = 0; i < history_length; i++)
    {
      std::cout << std::setw(5) << (i + 1) << "  " << hist_list[i]->line << std::endl;
    }
  }
  // Case 2: Flags (-w, -a, -r)
  else if (args.size() >= 2 && args[1][0] == '-')
  {
    std::string flag = args[1];
    std::string filename;

    // Get filename if provided, otherwise use default
    if (args.size() >= 3)
    {
      filename = args[2];
    }
    else
    {
      // Default to ~/.shell_history
      const char *home = getenv("HOME");
      if (home)
      {
        filename = std::string(home) + "/.shell_history";
      }
      else
      {
        filename = ".shell_history";
      }
    }

    // Handle different flags
    if (flag == "-w")
    {
      // Write history to file
      int result = write_history(filename.c_str());
      if (result != 0)
      {
        std::cerr << "history: write error: " << strerror(errno) << std::endl;
      }
      else
      {
        // Update offset to current history length
        history_offset = history_length;
      }
    }
    else if (flag == "-a")
    {
      // Append only NEW history entries to file
      int new_entries = history_length - history_offset;
      if (new_entries > 0)
      {
        int result = append_history(new_entries, filename.c_str());
        if (result != 0)
        {
          std::cerr << "history: append error: " << strerror(errno) << std::endl;
        }
        else
        {
          // Update offset to current history length
          history_offset = history_length;
        }
      }
    }
    else if (flag == "-r")
    {
      // Read history from file
      int result = read_history(filename.c_str());
      if (result != 0)
      {
        std::cerr << "history: read error: " << strerror(errno) << std::endl;
      }
    }
    else
    {
      std::cerr << "history: " << flag << ": invalid option" << std::endl;
    }
  }
  // Case 3: Number argument - show last N entries
  else if (args.size() == 2)
  {
    int limit = -1;
    try
    {
      limit = std::stoi(args[1]);
    }
    catch (...)
    {
      std::cerr << "history: " << args[1] << ": numeric argument required" << std::endl;
      return true;
    }

    if (limit <= 0)
    {
      return true; // Show nothing for 0 or negative
    }

    HIST_ENTRY **hist_list = history_list();
    if (hist_list == nullptr || history_length == 0)
    {
      return true;
    }

    int start_index = history_length - limit;
    if (start_index < 0)
    {
      start_index = 0;
    }

    for (int i = start_index; i < history_length; i++)
    {
      std::cout << std::setw(5) << (i + 1) << "  " << hist_list[i]->line << std::endl;
    }
  }

  return true;
}
