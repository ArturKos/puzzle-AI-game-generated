/**
 * @file scores.h
 * @brief High score persistence: load from and save to scores.txt.
 */

#pragma once

#include "types.h"
#include <vector>

/**
 * @brief Load all score entries from the scores file.
 *
 * Reads the pipe-delimited scores file. Each line has the format:
 * `name|timeSec|moves|difficulty`. Old entries without a difficulty
 * field default to 0 (Easy).
 *
 * @return Vector of all stored score entries. Empty if file doesn't exist.
 */
std::vector<ScoreEntry> loadScores();

/**
 * @brief Append a score entry to the scores file.
 * @param entry The score to save.
 */
void saveScore(const ScoreEntry& entry);
