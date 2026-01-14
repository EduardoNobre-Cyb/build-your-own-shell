#include "include/command_parser.h"
#include <iostream>

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
