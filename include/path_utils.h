#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <string>
#include <vector>

/**
 * Split PATH environment variable into individual directory paths
 * @param path_env The PATH environment variable string
 * @return Vector of directory paths
 */
std::vector<std::string> split_path(const std::string &path_env);

/**
 * Check if a file exists and is executable
 * @param filepath Path to the file to check
 * @return true if file is executable, false otherwise
 */
bool is_executable(const std::string &filepath);

/**
 * Find an executable program in the system PATH
 * @param program Name of the program to find
 * @return Full path to the program, or empty string if not found
 */
std::string find_in_path(const std::string &program);

#endif // PATH_UTILS_H
