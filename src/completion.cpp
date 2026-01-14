#include "include/completion.h"
#include "include/path_utils.h"
#include <vector>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

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
            const char *home = getenv("HOME");
            if (prefix[0] == '~' && home && search_dir.find(home) == 0)
            {
              if (full_path.substr(0, strlen(home)) == home)
              {
                completion = "~" + full_path.substr(strlen(home));
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
  if (completion_index < static_cast<int>(completion_matches.size()))
  {
    return strdup(completion_matches[completion_index++].c_str());
  }

  return nullptr;
}

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
  if (completion_index < static_cast<int>(completion_matches.size()))
  {
    return strdup(completion_matches[completion_index++].c_str());
  }

  return nullptr; // No more matches
}

char **command_completion(const char *text, int start, int end)
{
  (void)end; // Unused parameter
  
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

void init_completion()
{
  rl_attempted_completion_function = command_completion;
}
