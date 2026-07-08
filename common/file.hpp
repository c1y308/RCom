#ifndef COMMON_FILE_HPP
#define COMMON_FILE_HPP
#include <string>
/**
 * @brief Get absolute path by concatenating prefix and relative_path.
 * @return The absolute path.
 */
std::string get_absolute_path(const std::string &prefix,
                              const std::string &relative_path);

/**
 * @brief Check if the path exists.
 * @param path a file name, such as /a/b/c.txt
 * @return If the path exists.
 */
bool path_exists(const std::string &path);


//给定一个文件路径，获取文件名
std::string get_file_name(const std::string &path,
                          const bool remove_extension = false);

#endif  // COMMON_FILE_HPP
