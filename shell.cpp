#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <sstream>
#include <iomanip>    // for std::setw
#include <cstdlib>    // for getenv()
#include <unistd.h>   // for access(), dup2()
#include <sys/wait.h> // for waitpid()
#include <sys/stat.h> // for stat()
#include <fcntl.h>    // for open()
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h> // for directory reading
#include <cstring>  // for strdup
#include <cerrno>   // for errno

// Function to split PATH into directories
std::vector<std::string> split_path(const std::string &path_env)
{
  std::vector<std::string> paths;
  std::stringstream ss(path_env);
  std::string dir;

  while (std::getline(ss, dir, ':'))
  { // Split by ':' on Linux
    if (!dir.empty())
    {
      paths.push_back(dir);
    }
  }
  return paths;
}

// Check if file exists and is executable
bool is_executable(const std::string &filepath)
{
  return access(filepath.c_str(), X_OK) == 0;
}

std::string find_in_path(const std::string &program)
{
  const char *path_env = std::getenv("PATH");
  if (path_env == nullptr)
  {
    return "";
  }

  std::vector<std::string> paths = split_path(path_env);

  for (const std::string &dir : paths)
  {
    std::string full_path = dir + "/" + program;
    if (is_executable(full_path))
    {
      return full_path;
    }
  }

  return ""; // Not found
}

std::vector<std::string> parse_args(const std::string &command)
{
  std::vector<std::string> args;
  std::string crr_arg;

  enum State
  {
    NORMAL,
    IN_SINGLE_QUOTE,
    IN_DOUBLE_QUOTE,
    ESCAPED
  };
  State state = NORMAL;
  State prev_state = NORMAL; // Track previous state before ESCAPED

  // Loop through each character
  for (size_t i = 0; i < command.length(); i++)
  {
    char c = command[i];

    switch (state)
    {
    case NORMAL:
      if (c == ' ')
      {
        if (!crr_arg.empty())
        {
          args.push_back(crr_arg);
          crr_arg.clear();
        }
      }
      else if (c == '\'')
      {
        state = IN_SINGLE_QUOTE;
      }
      else if (c == '"')
      {
        state = IN_DOUBLE_QUOTE;
      }
      else if (c == '\\')
      {
        prev_state = NORMAL; // Remember we came from NORMAL
        state = ESCAPED;
      }
      else
      {
        crr_arg += c;
      }
      break;

    case IN_SINGLE_QUOTE:
      if (c == '\'')
      {
        state = NORMAL;
      }
      else
      {
        crr_arg += c;
      }
      break;

    case IN_DOUBLE_QUOTE:
      if (c == '"')
      {
        state = NORMAL;
      }
      else if (c == '\\')
      {
        // Look ahead to see if next char is escapable in double quotes
        if (i + 1 < command.length())
        {
          char next = command[i + 1];
          // Only escape these special chars inside double quotes: " \ $ `
          if (next == '"' || next == '\\' || next == '$' || next == '`')
          {
            prev_state = IN_DOUBLE_QUOTE;
            state = ESCAPED;
          }
          else
          {
            // Not escapable - keep the backslash literal
            crr_arg += c;
          }
        }
        else
        {
          // Backslash at end of string - keep it literal
          crr_arg += c;
        }
      }
      else
      {
        crr_arg += c;
      }
      break;

    case ESCAPED:
      crr_arg += c;       // Add escaped character literally
      state = prev_state; // Return to previous state
      break;
    }
  }

  // Add the last argument if not empty
  if (!crr_arg.empty())
  {
    args.push_back(crr_arg);
  }

  // Error checking - unmatched quotes
  if (state == IN_SINGLE_QUOTE || state == IN_DOUBLE_QUOTE)
  {
    std::cerr << "Error: Unmatched quotes in command." << std::endl;
  }

  return args;
}

// Struct to hold redirection info
struct RedirectInfo
{
  bool has_redirect;    // true if redirection found
  std::string command;  // command without redirection part
  std::string filename; // filename to redirect to
  bool redirect_stderr; // true if 2> or 2>>, false if 1> or > or 1>> or >>
  bool append_mode;     // true if >> or 1>> or 2>>, false if > or 1> or 2>
};

// Parse redirection from command
RedirectInfo parse_redirect(const std::string &full_command)
{
  RedirectInfo info; // Initialize with defaults assuming no redirection
  info.has_redirect = false;
  info.command = full_command;
  info.redirect_stderr = false;
  info.append_mode = false;

  // Find > outside of quotes
  bool in_single_quote = false;
  bool in_double_quote = false;
  bool escaped = false;

  for (size_t i = 0; i < full_command.length(); i++)
  {
    char c = full_command[i];

    if (escaped)
    {
      escaped = false;
      continue;
    }

    if (c == '\\' && !in_single_quote)
    {
      escaped = true;
      continue;
    }

    if (c == '\'' && !in_double_quote)
    {
      in_single_quote = !in_single_quote;
      continue;
    }

    if (c == '"' && !in_single_quote)
    {
      in_double_quote = !in_double_quote;
      continue;
    }

    // Found > outside quotes
    if (c == '>' && !in_single_quote && !in_double_quote)
    {
      // Save the position of the first >
      size_t redirect_start = i;

      // Check if it's >> (append mode)
      if (i + 1 < full_command.length() && full_command[i + 1] == '>')
      {
        info.append_mode = true;
        i++; // Skip the second >
      }

      // Check if it's 1> or 2> or 1>> or 2>>
      if (redirect_start > 0 && (full_command[redirect_start - 1] == '1' || full_command[redirect_start - 1] == '2'))
      {
        redirect_start = redirect_start - 1;
        // Make sure the number is not part of a word
        if (redirect_start > 0 && full_command[redirect_start - 1] != ' ')
        {
          continue; // Not a redirect, part of a word
        }

        // Check if it's 2> or 2>>
        if (full_command[redirect_start] == '2')
        {
          info.redirect_stderr = true;
        }
      }

      info.has_redirect = true;
      info.command = full_command.substr(0, redirect_start);

      // Trim trailing spaces from command
      while (!info.command.empty() && info.command.back() == ' ')
      {
        info.command.pop_back();
      }

      // Extract filename after > or >>
      std::string after_redirect = full_command.substr(i + 1);

      // Trim leading spaces
      size_t first_non_space = after_redirect.find_first_not_of(" \t");
      if (first_non_space != std::string::npos)
      {
        after_redirect = after_redirect.substr(first_non_space);
      }

      // Parse the filename (might have quotes)
      std::vector<std::string> redirect_args = parse_args(after_redirect);
      if (!redirect_args.empty())
      {
        info.filename = redirect_args[0];
      }
      break;
    }
  }

  return info;
}

// Parse command into pipeline segments (split by |)
std::vector<std::string> parse_pipeline(const std::string &command)
{
  std::vector<std::string> segments;
  std::string current_segment;

  // Track quote state to ignore | inside quotes
  bool in_single_quote = false;
  bool in_double_quote = false;
  bool escaped = false;

  for (size_t i = 0; i < command.length(); i++)
  {
    char c = command[i];

    if (escaped)
    {
      current_segment += c;
      escaped = false;
      continue;
    }

    if (c == '\\' && !in_single_quote)
    {
      current_segment += c;
      escaped = true;
      continue;
    }

    if (c == '\'' && !in_double_quote)
    {
      current_segment += c;
      in_single_quote = !in_single_quote;
      continue;
    }

    if (c == '"' && !in_single_quote)
    {
      current_segment += c;
      in_double_quote = !in_double_quote;
      continue;
    }

    // Found | outside quotes - this is a pipe!
    if (c == '|' && !in_single_quote && !in_double_quote)
    {
      // Trim leading/trailing spaces from segment
      size_t start = current_segment.find_first_not_of(" \t");
      size_t end = current_segment.find_last_not_of(" \t");

      if (start != std::string::npos)
      {
        segments.push_back(current_segment.substr(start, end - start + 1));
      }

      current_segment.clear();
      continue;
    }

    current_segment += c;
  }

  // Add last segment
  size_t start = current_segment.find_first_not_of(" \t");
  size_t end = current_segment.find_last_not_of(" \t");

  if (start != std::string::npos)
  {
    segments.push_back(current_segment.substr(start, end - start + 1));
  }

  return segments;
}

// Execute a pipeline of commands
void execute_pipeline(const std::vector<std::string> &commands, const std::set<std::string> &builtins)
{
  int num_commands = commands.size();

  if (num_commands == 0)
    return;

  // Create pipes: we need (n-1) pipes for n commands
  int pipes[num_commands - 1][2];
  for (int i = 0; i < num_commands - 1; i++)
  {
    if (pipe(pipes[i]) < 0)
    {
      std::cerr << "pipe failed" << std::endl;
      return;
    }
  }

  // Fork a process for each command
  for (int i = 0; i < num_commands; i++)
  {
    pid_t pid = fork();

    if (pid < 0)
    {
      std::cerr << "fork failed" << std::endl;
      return;
    }

    if (pid == 0)
    {
      // CHILD PROCESS

      // Redirect stdin from previous pipe (if not first command)
      if (i > 0)
      {
        dup2(pipes[i - 1][0], STDIN_FILENO);
      }

      // Redirect stdout to next pipe (if not last command)
      if (i < num_commands - 1)
      {
        dup2(pipes[i][1], STDOUT_FILENO);
      }

      // Close all pipe file descriptors in child
      for (int j = 0; j < num_commands - 1; j++)
      {
        close(pipes[j][0]);
        close(pipes[j][1]);
      }

      // Parse and execute this command
      std::string cmd = commands[i];
      RedirectInfo redir = parse_redirect(cmd);
      std::vector<std::string> args = parse_args(redir.command);

      if (args.empty())
      {
        exit(1);
      }

      // Handle redirection if present
      if (redir.has_redirect)
      {
        int flags = O_WRONLY | O_CREAT;
        if (redir.append_mode)
        {
          flags |= O_APPEND;
        }
        else
        {
          flags |= O_TRUNC;
        }

        int fd = open(redir.filename.c_str(), flags, 0644);
        if (fd < 0)
        {
          std::cerr << "Error: cannot open file" << std::endl;
          exit(1);
        }

        if (redir.redirect_stderr)
        {
          dup2(fd, STDERR_FILENO);
        }
        else
        {
          dup2(fd, STDOUT_FILENO);
        }
        close(fd);
      }

      // Check if it's a builtin - builtins in pipelines must run in child process
      if (args[0] == "echo")
      {
        // Execute echo in child
        for (size_t j = 1; j < args.size(); j++)
        {
          if (j > 1)
            std::cout << " ";
          std::cout << args[j];
        }
        std::cout << std::endl;
        exit(0);
      }
      else if (args[0] == "pwd")
      {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
        {
          std::cout << cwd << std::endl;
        }
        exit(0);
      }
      else if (args[0] == "type")
      {
        // Execute type in child
        if (args.size() > 1)
        {
          std::string arg = args[1];
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
        exit(0);
      }
      else if (args[0] == "cd")
      {
        // cd in pipeline doesn't make sense but handle it anyway
        std::cerr << "cd: cannot change directory in pipeline" << std::endl;
        exit(1);
      }
      else if (args[0] == "exit")
      {
        // exit in pipeline shouldn't exit the shell
        std::cerr << "exit: cannot exit from pipeline" << std::endl;
        exit(1);
      }
      else
      {
        // External command
        std::string path = find_in_path(args[0]);
        if (path.empty())
        {
          std::cerr << args[0] << ": command not found" << std::endl;
          exit(1);
        }

        std::vector<char *> c_args;
        for (const auto &arg : args)
        {
          c_args.push_back(const_cast<char *>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execv(path.c_str(), c_args.data());
        std::cerr << "execv failed" << std::endl;
        exit(1);
      }
    }
  }

  // PARENT PROCESS
  // Close all pipe file descriptors in parent
  for (int i = 0; i < num_commands - 1; i++)
  {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }

  // Wait for all children
  for (int i = 0; i < num_commands; i++)
  {
    wait(nullptr);
  }
}

void execute_command(const std::string &path,
                     const std::vector<std::string> &args,
                     const std::string &redirect_file = "",
                     bool redirect_stderr = false,
                     bool append_mode = false)
{
  pid_t pid = fork(); // Create child process

  if (pid == 0)
  {
    // CHILD PROCESS - runs the command

    // Handle output redirection
    if (!redirect_file.empty())
    {
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

      int fd = open(redirect_file.c_str(), flags, 0644);
      if (fd < 0)
      {
        std::cerr << "Error: cannot open file for writing" << std::endl;
        exit(1);
      }

      // Redirect either stdout or stderr based on redirect type
      if (redirect_stderr)
      {
        dup2(fd, STDERR_FILENO); // Redirect stderr (fd 2)
      }
      else
      {
        dup2(fd, STDOUT_FILENO); // Redirect stdout (fd 1)
      }
      close(fd);
    }

    // Convert args to char* array (exec requires this format)
    std::vector<char *> c_args;
    for (const auto &arg : args)
    {
      c_args.push_back(const_cast<char *>(arg.c_str()));
    }
    c_args.push_back(nullptr); // Null-terminate the array

    execv(path.c_str(), c_args.data());

    // If execv returns, it failed
    std::cerr << "Failed to execute " << path << std::endl;
    exit(1);
  }
  else if (pid > 0)
  {
    // PARENT PROCESS - wait for child to finish
    int status;
    waitpid(pid, &status, 0);
  }
  else
  {
    // Fork failed
    std::cerr << "Failed to create process for " << path << std::endl;
  }
}

// List of builtin commands for completion
static std::vector<std::string> builtin_commands = {
    "echo",
    "type",
    "exit",
    "pwd",
    "cd",
    "history",
};
static std::vector<std::string> completion_matches;
static int completion_index = 0;

// Generator function for directory completion (for cd command)
char *directory_generator(const char *text, int state)
{
  // state = 0 means this is a new word to complete
  if (state == 0)
  {
    completion_matches.clear();
    completion_index = 0;

    std::string prefix(text);
    std::string search_dir = ".";
    std::string search_prefix = prefix;

    // Handle ~ expansion
    if (prefix.length() > 0 && prefix[0] == '~')
    {
      const char *home = getenv("HOME");
      if (home)
      {
        if (prefix == "~" || (prefix.length() > 1 && prefix[1] == '/'))
        {
          search_dir = home;
          if (prefix.length() > 2)
          {
            search_dir += prefix.substr(1);
          }
          // Find last /
          size_t last_slash = search_dir.find_last_of('/');
          if (last_slash != std::string::npos && last_slash > strlen(home) - 1)
          {
            search_prefix = search_dir.substr(last_slash + 1);
            search_dir = search_dir.substr(0, last_slash);
          }
          else
          {
            search_prefix = "";
          }
        }
      }
    }
    else if (prefix.find('/') != std::string::npos)
    {
      // Has path separator
      size_t last_slash = prefix.find_last_of('/');
      search_dir = prefix.substr(0, last_slash);
      if (search_dir.empty())
        search_dir = "/";
      search_prefix = prefix.substr(last_slash + 1);
    }

    // List directories
    DIR *dp = opendir(search_dir.c_str());
    if (dp)
    {
      struct dirent *entry;
      while ((entry = readdir(dp)) != nullptr)
      {
        std::string name(entry->d_name);
        if (name == "." || name == "..") // Skip . and ..
          continue;

        if (name.substr(0, search_prefix.length()) == search_prefix)
        {
          std::string full_path = search_dir + "/" + name;
          struct stat st;
          if (stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
          {
            // Build the completion string
            std::string completion;
            if (prefix[0] == '~' && search_dir.find(getenv("HOME") ? getenv("HOME") : "") == 0)
            {
              const char *home = getenv("HOME");
              if (home && full_path.substr(0, strlen(home)) == home)
              {
                completion = "~" + full_path.substr(strlen(home)) + "/" + name;
              }
              else
              {
                completion = name;
              }
            }
            else if (search_dir != ".")
            {
              completion = search_dir + "/" + name;
            }
            else
            {
              completion = name;
            }
            completion_matches.push_back(completion);
          }
        }
      }
      closedir(dp);
    }
  }

  // Return next match
  if (completion_index < completion_matches.size())
  {
    return strdup(completion_matches[completion_index++].c_str());
  }

  return nullptr;
}

// Generator function for command completion
char *command_generator(const char *text, int state)
{
  // state = 0 means this is a new word to complete
  if (state == 0)
  {
    completion_matches.clear();
    completion_index = 0;

    std::string prefix(text);

    // Find builtins that match
    for (const auto &cmd : builtin_commands)
    {
      if (cmd.substr(0, prefix.length()) == prefix)
      {
        completion_matches.push_back(cmd);
      }
    }

    // Find executables in PATH that match
    const char *path_env = std::getenv("PATH");
    if (path_env)
    {
      std::vector<std::string> paths = split_path(path_env);
      for (const auto &dir : paths)
      {
        DIR *dp = opendir(dir.c_str());
        if (dp)
        {
          struct dirent *entry;
          while ((entry = readdir(dp)) != nullptr)
          {
            std::string name(entry->d_name);
            if (name.substr(0, prefix.length()) == prefix)
            {
              std::string full_path = dir + "/" + name;
              if (access(full_path.c_str(), X_OK) == 0)
              {
                completion_matches.push_back(name);
              }
            }
          }
          closedir(dp);
        }
      }
    }
  }

  // Return next match
  if (completion_index < completion_matches.size())
  {
    return strdup(completion_matches[completion_index++].c_str());
  }

  return nullptr; // No more matches
}

// Attempt command completion
char **command_completion(const char *text, int start, int end)
{
  // First word - complete command names
  if (start == 0)
  {
    return rl_completion_matches(text, command_generator);
  }

  // Check if the command is "cd" - then complete directories only
  std::string line(rl_line_buffer);
  size_t first_space = line.find(' ');
  if (first_space != std::string::npos)
  {
    std::string cmd = line.substr(0, first_space);
    if (cmd == "cd")
    {
      // Complete directories only for cd command
      return rl_completion_matches(text, directory_generator);
    }
  }

  // For other arguments, complete filenames (Readline's default)
  return rl_completion_matches(text, rl_filename_completion_function);
}

int main()
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::set<std::string> builtins = {"echo", "type", "exit", "pwd", "cd", "history"};

  // Track history offset for append operations
  int history_offset = 0;

  // Get history file path from HISTFILE env variable or use default
  std::string histfile;
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

  // Load history from file on startup
  read_history(histfile.c_str());
  history_offset = history_length;

  // Set up readline
  rl_attempted_completion_function = command_completion;

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

    if (input == nullptr) // EOF (CRTL+D)
    {
      // Append new history entries before exiting
      int new_entries = history_length - history_offset;
      if (new_entries > 0)
      {
        append_history(new_entries, histfile.c_str());
      }
      break;
    }

    std::string command(input);

    if (!command.empty())
    {
      add_history(input); // Add to history for up/down arrow nav
    }

    free(input); // readline allocates memory, free it after use

    // Check if command contains a pipeline
    if (command.find('|') != std::string::npos)
    {
      // Parse pipeline and execute
      std::vector<std::string> pipeline_commands = parse_pipeline(command);
      execute_pipeline(pipeline_commands, builtins);
      continue;
    }

    // Parse command
    size_t space_pos = command.find(' ');
    std::string cmd = (space_pos != std::string::npos) ? command.substr(0, space_pos) : command;

    // Check for exit
    if (command == "exit")
    {
      // Append new history entries before exiting
      int new_entries = history_length - history_offset;
      if (new_entries > 0)
      {
        append_history(new_entries, histfile.c_str());
      }
      break;
    }
    else if (command.substr(0, 5) == "echo ")
    {
      // Check for redirection
      RedirectInfo redir = parse_redirect(command);

      int saved_fd = -1;
      if (redir.has_redirect)
      {
        // Save original fd (stdout or stderr)
        int fd_to_redirect = redir.redirect_stderr ? STDERR_FILENO : STDOUT_FILENO;
        saved_fd = dup(fd_to_redirect);

        // Choose flags based on append mode
        int flags = O_WRONLY | O_CREAT;
        if (redir.append_mode)
        {
          flags |= O_APPEND; // Append to end of file
        }
        else
        {
          flags |= O_TRUNC; // Truncate (overwrite) file
        }

        // Redirect to file
        int fd = open(redir.filename.c_str(), flags, 0644);
        if (fd < 0)
        {
          std::cerr << "Error: cannot open file for writing" << std::endl;
          continue;
        }
        dup2(fd, fd_to_redirect);
        close(fd);
      }

      // Parse arguments with quote handling
      std::vector<std::string> args = parse_args(redir.command);

      // Print all arguments except the first one (which is "echo")
      for (size_t i = 1; i < args.size(); i++)
      {
        if (i > 1)
          std::cout << " "; // Add space between arguments
        std::cout << args[i];
      }
      std::cout << std::endl;

      if (redir.has_redirect)
      {
        // Restore original fd
        int fd_to_redirect = redir.redirect_stderr ? STDERR_FILENO : STDOUT_FILENO;
        dup2(saved_fd, fd_to_redirect);
        close(saved_fd);
      }
      continue;
    }
    else if (command == "pwd")
    {
      char cwd[1024]; // Buffer for current working directory
      if (getcwd(cwd, sizeof(cwd)) == nullptr)
      {
        std::cerr << "pwd: error retrieving current directory" << std::endl; // FAIL: handle error
      }
      else
      {
        std::cout << cwd << std::endl; // SUCCESS: print current directory
      }
    }
    else if (command == "cd" || command.substr(0, 3) == "cd ")
    {
      std::string path;
      if (command == "cd")
      {
        // No argument - go to HOME
        const char *home = std::getenv("HOME");
        if (home != nullptr)
        {
          path = home;
        }
        else
        {
          std::cerr << "cd: HOME not set" << std::endl;
          continue; // Skip to next iteration
        }
      }
      else
      {
        // Has argument - extract it
        path = command.substr(3); // Skip "cd "

        // Handle tilde expansion
        if (path == "~" || path.substr(0, 2) == "~/")
        {
          const char *home = std::getenv("HOME");
          if (home != nullptr)
          {
            if (path == "~")
            {
              path = home;
            }
            else
            {
              path = std::string(home) + path.substr(1); // Replace ~ with HOME
            }
          }
        }
      }

      // Change directory
      if (chdir(path.c_str()) != 0)
      {
        std::cerr << "cd: " << path << ": No such file or directory" << std::endl;
      }
    }
    else if (command.substr(0, 5) == "type ")
    {
      std::string arg = command.substr(5);
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
    else if (command == "history" || command.substr(0, 8) == "history ")
    {
      // Parse arguments
      std::vector<std::string> parts = parse_args(command);

      // Case 1: Just "history" - show all
      if (parts.size() == 1)
      {
        HIST_ENTRY **hist_list = history_list();
        if (hist_list == nullptr || history_length == 0)
        {
          continue;
        }

        for (int i = 0; i < history_length; i++)
        {
          std::cout << std::setw(5) << (i + 1) << "  " << hist_list[i]->line << std::endl;
        }
      }
      // Case 2: Flags (-w, -a, -r)
      else if (parts.size() >= 2 && parts[1][0] == '-')
      {
        std::string flag = parts[1];
        std::string filename;

        // Get filename if provided, otherwise use default
        if (parts.size() >= 3)
        {
          filename = parts[2];
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
      else if (parts.size() == 2)
      {
        int limit = -1;
        try
        {
          limit = std::stoi(parts[1]);
        }
        catch (...)
        {
          std::cerr << "history: " << parts[1] << ": numeric argument required" << std::endl;
          continue;
        }

        if (limit <= 0)
        {
          continue; // Show nothing for 0 or negative
        }

        HIST_ENTRY **hist_list = history_list();
        if (hist_list == nullptr || history_length == 0)
        {
          continue;
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
    }

    else
    {
      // Try to execute external command
      RedirectInfo redir = parse_redirect(command);
      std::vector<std::string> args = parse_args(redir.command);

      if (!args.empty())
      {
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
    }
  } // End of while loop

  return 0;
}
