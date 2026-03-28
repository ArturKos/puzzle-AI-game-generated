/**
 * @file flying_pieces.h
 * @brief Decorative floating puzzle piece animations for intro and win screens.
 */

#pragma once

#include "types.h"
#include <SFML/Graphics.hpp>
#include <vector>

/**
 * @brief Initialize a set of flying pieces with random positions and velocities.
 *
 * Each flying piece gets a random position, velocity, rotation, scale, and
 * texture index, creating a visually varied floating animation.
 *
 * @param fp     Vector to populate (resized to @p count).
 * @param count  Number of flying pieces to create.
 * @param winW   Window width for position bounds.
 * @param winH   Window height for position bounds.
 * @param numTex Number of available piece textures (for random selection).
 */
void initFlying(std::vector<FlyingPiece>& fp, int count,
                int winW, int winH, int numTex);

/**
 * @brief Update flying piece positions and rotations by one frame.
 *
 * Pieces wrap around screen edges to create an infinite floating effect.
 *
 * @param fp  Vector of flying pieces to update.
 * @param dt  Frame delta time in seconds.
 * @param W   Window width for wrapping.
 * @param H   Window height for wrapping.
 */
void updateFlying(std::vector<FlyingPiece>& fp, float dt, int W, int H);

/**
 * @brief Draw all flying pieces to the render window.
 *
 * Each flying piece is drawn centered, rotated, scaled, and semi-transparent
 * using the texture from the corresponding puzzle piece.
 *
 * @param window Target render window.
 * @param fp     Flying pieces to draw.
 * @param pieces Puzzle pieces providing the textures.
 */
void drawFlying(sf::RenderWindow& window,
                const std::vector<FlyingPiece>& fp,
                const std::vector<Piece>& pieces);
