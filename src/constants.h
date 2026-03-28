/**
 * @file constants.h
 * @brief Global constants and configuration values for the Jigsaw Puzzle game.
 */

#pragma once

#include <string>

inline constexpr float TAB_RATIO       = 0.14f;    ///< Tab size relative to cell dimension.
inline constexpr float SNAP_DIST       = 18.f;     ///< Max pixel distance for snap-to-grid.
inline constexpr int   SIDEBAR_W       = 220;       ///< Sidebar width in pixels.
inline constexpr int   BOARD_PADDING   = 30;        ///< Padding around the puzzle board.
inline constexpr float SHADOW_OFFSET   = 3.f;       ///< Drop shadow offset for unplaced pieces.
inline constexpr float OUTLINE_WIDTH   = 1.5f;      ///< Piece outline stroke width.
inline constexpr int   MAX_NAME_LEN    = 16;        ///< Maximum player name length.
inline constexpr float INTRO_DURATION  = 4.5f;      ///< Intro screen auto-advance timeout (seconds).
inline constexpr int   NUM_FLYING      = 18;        ///< Number of flying pieces on intro/win screens.

inline const std::string SCORES_FILE = "scores.txt";  ///< File path for persistent score storage.
inline const std::string CHEAT_CODE  = "adult";        ///< Cheat code keyword for adult image mode.
