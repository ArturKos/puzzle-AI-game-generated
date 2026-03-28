/**
 * @file puzzle_generator.h
 * @brief Puzzle grid generation: edge connectivity and piece creation.
 */

#pragma once

#include "types.h"
#include <SFML/Graphics.hpp>
#include <vector>

/**
 * @brief Generate a complete set of jigsaw pieces for the given grid size.
 *
 * Creates interlocking edge connectivity (randomized tabs/blanks with flat
 * borders), renders each piece texture via Cairo, and sets up home positions.
 *
 * @param[in]  srcImage  The source image to slice into pieces.
 * @param[in]  gridCols  Number of columns in the grid.
 * @param[in]  gridRows  Number of rows in the grid.
 * @param[out] pieces    Output vector of generated pieces (resized).
 * @param[out] drawOrder Output draw order indices (resized, 0..N-1).
 * @param[out] cellW     Computed cell width in pixels.
 * @param[out] cellH     Computed cell height in pixels.
 * @param[out] pad       Computed padding (tab overhang + margin).
 */
void generatePuzzle(const sf::Image& srcImage,
                    int gridCols, int gridRows,
                    std::vector<Piece>& pieces,
                    std::vector<int>& drawOrder,
                    int& cellW, int& cellH, float& pad);
