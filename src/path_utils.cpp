#include "include/path_utils.h"
#include <sstream>
#include <unistd.h>

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
