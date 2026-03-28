/**
 * @file types.h
 * @brief Core data types and enumerations for the Jigsaw Puzzle game.
 */

#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string>

/// Puzzle edge type: flat border, outward tab, or inward blank.
enum Edge { EDGE_FLAT = 0, EDGE_TAB = 1, EDGE_BLANK = -1 };

/// Game state machine states.
enum GameState {
    STATE_INTRO,       ///< Animated intro screen with flying pieces.
    STATE_NAME_INPUT,  ///< Player name entry and difficulty selection.
    STATE_PLAYING,     ///< Active puzzle gameplay.
    STATE_WIN_SCREEN,  ///< Puzzle completion celebration.
    STATE_SCORES,      ///< High scores leaderboard.
};

/// Difficulty levels.
enum Difficulty { DIFF_EASY = 0, DIFF_MEDIUM = 1, DIFF_NIGHTMARE = 2 };

/// Difficulty level configuration.
struct DiffInfo {
    const char* label;  ///< Display name.
    int cols, rows;     ///< Grid dimensions.
    sf::Color color;    ///< UI accent color.
};

/// Difficulty presets: Easy (4x3), Medium (8x6), Nightmare (16x12).
inline const DiffInfo DIFFICULTIES[] = {
    { "Easy",       4,  3, sf::Color( 80, 200, 120) },
    { "Medium",     8,  6, sf::Color(220, 180,  50) },
    { "Nightmare", 16, 12, sf::Color(220,  60,  60) },
};

/// A single jigsaw puzzle piece with position, edges, and rendering data.
struct Piece {
    int   col, row;              ///< Grid column and row.
    Edge  top, right, bottom, left; ///< Edge types for each side.
    float x, y;                  ///< Current screen position (cell top-left).
    float homeX, homeY;          ///< Correct target position for snapping.
    bool  placed   = false;      ///< Whether piece is locked in place.
    bool  dragging = false;      ///< Whether piece is being dragged.
    sf::Texture texture;         ///< Rendered piece texture (Cairo output).
    sf::Sprite  sprite;          ///< SFML sprite for drawing.
};

/// A decorative floating piece for intro/win screen animations.
struct FlyingPiece {
    float x, y;         ///< Position.
    float vx, vy;       ///< Velocity.
    float rot, rotSpeed; ///< Rotation angle and speed (degrees/sec).
    float scale;         ///< Uniform scale factor.
    int   texIdx;        ///< Index into the pieces texture array.
    uint8_t alpha;       ///< Transparency (0-255).
};

/// A single high score entry.
struct ScoreEntry {
    std::string name;   ///< Player name (cheat code stripped).
    int timeSec;        ///< Completion time in seconds.
    int moves;          ///< Total moves (drops).
    int difficulty;     ///< Difficulty level index (0/1/2).
};
