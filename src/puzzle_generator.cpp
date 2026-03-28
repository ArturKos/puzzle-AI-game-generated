/**
 * @file puzzle_generator.cpp
 * @brief Puzzle grid generation implementation.
 */

#include "puzzle_generator.h"
#include "constants.h"
#include "puzzle_renderer.h"

#include <array>
#include <cstdlib>
#include <numeric>

void generatePuzzle(const sf::Image& srcImage,
                    int gridCols, int gridRows,
                    std::vector<Piece>& pieces,
                    std::vector<int>& drawOrder,
                    int& cellW, int& cellH, float& pad)
{
    int imgW = static_cast<int>(srcImage.getSize().x);
    int imgH = static_cast<int>(srcImage.getSize().y);
    cellW = imgW / gridCols;
    cellH = imgH / gridRows;

    float tabW = cellW * TAB_RATIO;
    float tabH = cellH * TAB_RATIO;
    pad = std::max(tabW, tabH) + 4.f;

    // Generate interlocking edge connectivity
    std::vector<std::vector<std::array<Edge, 4>>> edges(
        gridRows, std::vector<std::array<Edge, 4>>(gridCols));

    for (int r = 0; r < gridRows; ++r)
        for (int c = 0; c < gridCols; ++c) {
            edges[r][c][0] = (r == 0) ? EDGE_FLAT
                           : static_cast<Edge>(-edges[r - 1][c][2]);
            edges[r][c][3] = (c == 0) ? EDGE_FLAT
                           : static_cast<Edge>(-edges[r][c - 1][1]);
            edges[r][c][1] = (c == gridCols - 1) ? EDGE_FLAT
                           : ((std::rand() % 2) ? EDGE_TAB : EDGE_BLANK);
            edges[r][c][2] = (r == gridRows - 1) ? EDGE_FLAT
                           : ((std::rand() % 2) ? EDGE_TAB : EDGE_BLANK);
        }

    // Create and render each piece
    int total = gridCols * gridRows;
    pieces.resize(total);
    for (int r = 0; r < gridRows; ++r)
        for (int c = 0; c < gridCols; ++c) {
            int idx = r * gridCols + c;
            Piece& p = pieces[idx];
            p.col    = c;
            p.row    = r;
            p.top    = edges[r][c][0];
            p.right  = edges[r][c][1];
            p.bottom = edges[r][c][2];
            p.left   = edges[r][c][3];
            p.homeX  = BOARD_PADDING + c * cellW;
            p.homeY  = BOARD_PADDING + r * cellH;
            p.placed = false;
            p.dragging = false;
            p.texture = renderPiece(srcImage, c, r, cellW, cellH,
                                    p.top, p.right, p.bottom, p.left);
            p.sprite.setTexture(p.texture, true);
        }

    drawOrder.resize(total);
    std::iota(drawOrder.begin(), drawOrder.end(), 0);
}
