#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>
#include <vector>

/**
 * Parse a command string into individual arguments, handling quotes and escapes
 * @param command The command string to parse
 * @return Vector of parsed arguments
 */
std::vector<std::string> parse_args(const std::string &command);

/**
 * Struct to hold information about output redirection
 */
struct RedirectInfo
{
  bool has_redirect;    // true if redirection found
  std::string command;  // command without redirection part
  std::string filename; // filename to redirect to
  bool redirect_stderr; // true if 2> or 2>>, false if 1> or > or 1>> or >>
  bool append_mode;     // true if >> or 1>> or 2>>, false if > or 1> or 2>
};

/**
 * Parse redirection operators from a command
 * @param full_command The full command string potentially containing redirection
 * @return RedirectInfo structure with parsed redirection information
 */
RedirectInfo parse_redirect(const std::string &full_command);

/**
 * Parse a command into pipeline segments (split by |)
 * @param command The command string potentially containing pipes
 * @return Vector of command segments separated by pipes
 */
std::vector<std::string> parse_pipeline(const std::string &command);

#endif // COMMAND_PARSER_H
