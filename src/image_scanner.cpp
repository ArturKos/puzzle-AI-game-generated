/**
 * @file image_scanner.cpp
 * @brief Image discovery from embedded resources.
 */

#include "image_scanner.h"
#include "embedded_resources.h"

#include <algorithm>
#include <string>

std::vector<std::string> scanImages(const std::string& dir) {
    // Convert "RES/img_kids" -> "img_kids/" prefix for embedded lookup
    std::string prefix = dir;
    if (prefix.rfind("RES/", 0) == 0)
        prefix = prefix.substr(4);
    if (!prefix.empty() && prefix.back() != '/')
        prefix += '/';

    return listEmbeddedResources(prefix);
}
