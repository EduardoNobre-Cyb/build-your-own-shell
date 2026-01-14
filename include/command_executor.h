#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include <string>
#include <vector>
#include <set>

/**
 * Execute an external command with optional output redirection
 * @param path Full path to the executable
 * @param args Vector of command arguments (including program name as first element)
 * @param redirect_file Filename to redirect output to (empty for no redirection)
 * @param redirect_stderr If true, redirect stderr; if false, redirect stdout
 * @param append_mode If true, append to file; if false, truncate file
 */
void execute_command(const std::string &path,
                     const std::vector<std::string> &args,
                     const std::string &redirect_file = "",
                     bool redirect_stderr = false,
                     bool append_mode = false);

/**
 * Execute a pipeline of commands
 * @param commands Vector of command strings to execute in a pipeline
 * @param builtins Set of builtin command names
 */
void execute_pipeline(const std::vector<std::string> &commands,
                      const std::set<std::string> &builtins);

#endif // COMMAND_EXECUTOR_H
