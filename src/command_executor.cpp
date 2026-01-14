#include "include/command_executor.h"
#include "include/command_parser.h"
#include "include/path_utils.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>

void execute_command(const std::string &path,
                     const std::vector<std::string> &args,
                     const std::string &redirect_file,
                     bool redirect_stderr,
                     bool append_mode)
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
