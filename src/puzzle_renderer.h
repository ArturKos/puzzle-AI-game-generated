/**
 * @file puzzle_renderer.h
 * @brief Cairo-based jigsaw piece shape generation and texture rendering.
 *
 * This module is the only part of the codebase that depends on Cairo.
 * It generates classic jigsaw piece outlines using cubic Bezier curves
 * and renders image-textured pieces with outlines and clipping.
 */

#pragma once

#include "types.h"
#include <cairo/cairo.h>
#include <SFML/Graphics.hpp>

/**
 * @brief Build a jigsaw piece outline path on a Cairo context.
 *
 * Draws a closed path with interlocking tabs/blanks on each edge using
 * 3 cubic Bezier curves per non-flat edge (narrow neck + round knob).
 * The path is offset by a padding margin to accommodate protruding tabs.
 *
 * @param cr     Active Cairo drawing context.
 * @param cellW  Piece cell width in pixels (excluding tab padding).
 * @param cellH  Piece cell height in pixels (excluding tab padding).
 * @param top    Edge type for the top side.
 * @param right  Edge type for the right side.
 * @param bottom Edge type for the bottom side.
 * @param left   Edge type for the left side.
 * @param tabW   Horizontal tab protrusion size.
 * @param tabH   Vertical tab protrusion size.
 */
void buildPuzzlePath(cairo_t* cr, int cellW, int cellH,
                     Edge top, Edge right, Edge bottom, Edge left,
                     float tabW, float tabH);

/**
 * @brief Render a single jigsaw piece as an SFML texture.
 *
 * Creates a Cairo surface, clips the source image to the puzzle piece shape,
 * draws a subtle outline, and converts the result to an sf::Texture.
 *
 * @param srcImage The full puzzle source image.
 * @param col      Grid column of this piece.
 * @param row      Grid row of this piece.
 * @param cellW    Cell width in pixels.
 * @param cellH    Cell height in pixels.
 * @param top      Edge type for the top side.
 * @param right    Edge type for the right side.
 * @param bottom   Edge type for the bottom side.
 * @param left     Edge type for the left side.
 * @return         SFML texture containing the rendered piece with transparency.
 */
sf::Texture renderPiece(const sf::Image& srcImage,
                        int col, int row, int cellW, int cellH,
                        Edge top, Edge right, Edge bottom, Edge left);
