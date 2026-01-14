#ifndef COMPLETION_H
#define COMPLETION_H

#include <readline/readline.h>

/**
 * Initialize readline completion system
 */
void init_completion();

/**
 * Generator function for command name completion
 * @param text The partial text to complete
 * @param state The completion state (0 for first call, non-zero for subsequent)
 * @return Next matching completion or nullptr when done
 */
char *command_generator(const char *text, int state);

/**
 * Generator function for directory name completion (for cd command)
 * @param text The partial text to complete
 * @param state The completion state (0 for first call, non-zero for subsequent)
 * @return Next matching completion or nullptr when done
 */
char *directory_generator(const char *text, int state);

/**
 * Main completion function called by readline
 * @param text The partial text to complete
 * @param start Start position in the line buffer
 * @param end End position in the line buffer
 * @return Array of possible completions
 */
char **command_completion(const char *text, int start, int end);

#endif // COMPLETION_H
