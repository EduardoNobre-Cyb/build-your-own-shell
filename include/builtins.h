#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <vector>
#include <set>

/**
 * Execute the echo builtin command
 * @param args Vector of arguments (including "echo" as first element)
 * @param redirect_file Filename to redirect output to (empty for no redirection)
 * @param redirect_stderr If true, redirect stderr; if false, redirect stdout
 * @param append_mode If true, append to file; if false, truncate file
 */
void builtin_echo(const std::vector<std::string> &args,
                  const std::string &redirect_file = "",
                  bool redirect_stderr = false,
                  bool append_mode = false);

/**
 * Execute the pwd builtin command
 */
void builtin_pwd();

/**
 * Execute the cd builtin command
 * @param path Directory to change to (empty string means HOME)
 * @return true if successful, false otherwise
 */
bool builtin_cd(const std::string &path);

/**
 * Execute the type builtin command
 * @param arg Command name to check
 * @param builtins Set of builtin command names
 */
void builtin_type(const std::string &arg, const std::set<std::string> &builtins);

/**
 * Execute the history builtin command
 * @param args Vector of arguments (including "history" as first element)
 * @param history_offset Reference to the history offset counter
 * @return true if shell should continue, false if should exit
 */
bool builtin_history(const std::vector<std::string> &args, int &history_offset);

#endif // BUILTINS_H
