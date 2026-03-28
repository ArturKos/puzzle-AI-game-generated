/**
 * @file image_scanner.h
 * @brief Filesystem scanning for puzzle image files.
 */

#pragma once

#include <string>
#include <vector>

/**
 * @brief Scan a directory for supported image files.
 *
 * Searches the given directory (non-recursively) for files with
 * .png, .jpg, .jpeg, or .bmp extensions. Results are sorted alphabetically.
 *
 * @param dir Path to the directory to scan.
 * @return Sorted vector of absolute file paths. Empty if directory doesn't exist.
 */
std::vector<std::string> scanImages(const std::string& dir);
