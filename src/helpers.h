/**
 * @file helpers.h
 * @brief Utility functions: formatting, random numbers, cheat code handling.
 */

#pragma once

#include <string>

/**
 * @brief Format seconds as MM:SS string.
 * @param totalSec Time in seconds.
 * @return Formatted string, e.g. "02:35".
 */
std::string formatTime(int totalSec);

/**
 * @brief Generate a random float in [lo, hi].
 * @param lo Lower bound (inclusive).
 * @param hi Upper bound (inclusive).
 * @return Random float value.
 */
float randf(float lo, float hi);

/**
 * @brief Get the display name for a difficulty level.
 * @param d Difficulty index (0=Easy, 1=Medium, 2=Nightmare).
 * @return Human-readable difficulty name.
 */
const char* diffName(int d);

/**
 * @brief Check if a player name contains the cheat code.
 *
 * Performs case-insensitive search for the CHEAT_CODE constant.
 *
 * @param name Player name to check.
 * @return true if the cheat code is found.
 */
bool containsCheatCode(const std::string& name);

/**
 * @brief Remove the cheat code from a player name.
 *
 * Strips all case-insensitive occurrences of the cheat code keyword
 * and trims leading/trailing whitespace from the result.
 *
 * @param name Original player name.
 * @return Cleaned name with cheat code removed.
 */
std::string stripCheatCode(const std::string& name);
