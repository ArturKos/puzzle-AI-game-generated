/**
 * @file puzzle_renderer.cpp
 * @brief Cairo-based jigsaw piece rendering implementation.
 */

#include "puzzle_renderer.h"
#include "constants.h"

#include <cmath>
#include <cstdint>
#include <vector>

void buildPuzzlePath(cairo_t* cr, int cellW, int cellH,
                     Edge top, Edge right, Edge bottom, Edge left,
                     float tabW, float tabH)
{
    const float w = static_cast<float>(cellW);
    const float h = static_cast<float>(cellH);

    auto drawEdge = [&](float x0, float y0, float x1, float y1,
                        Edge e, bool horizontal)
    {
        if (e == EDGE_FLAT) { cairo_line_to(cr, x1, y1); return; }

        float len  = horizontal ? std::abs(x1 - x0) : std::abs(y1 - y0);
        float dir  = horizontal ? ((x1 > x0) ? 1.f : -1.f)
                                : ((y1 > y0) ? 1.f : -1.f);
        float s    = static_cast<float>(e);

        float neckW   = len * 0.06f;
        float headW   = len * 0.11f;
        float height  = (horizontal ? tabH : tabW);
        float neckH   = height * 0.38f;
        float headTop = height * 1.18f;

        if (horizontal) {
            float mx = (x0 + x1) / 2.f, ey = y0;
            cairo_line_to(cr, mx - dir * neckW, ey);
            cairo_curve_to(cr, mx-dir*neckW, ey-s*neckH,
                mx-dir*headW, ey-s*(height*0.6f), mx-dir*headW, ey-s*height);
            cairo_curve_to(cr, mx-dir*headW, ey-s*headTop,
                mx+dir*headW, ey-s*headTop, mx+dir*headW, ey-s*height);
            cairo_curve_to(cr, mx+dir*headW, ey-s*(height*0.6f),
                mx+dir*neckW, ey-s*neckH, mx+dir*neckW, ey);
            cairo_line_to(cr, x1, y1);
        } else {
            float my = (y0 + y1) / 2.f, ex = x0;
            cairo_line_to(cr, ex, my - dir * neckW);
            cairo_curve_to(cr, ex+s*neckH, my-dir*neckW,
                ex+s*(height*0.6f), my-dir*headW, ex+s*height, my-dir*headW);
            cairo_curve_to(cr, ex+s*headTop, my-dir*headW,
                ex+s*headTop, my+dir*headW, ex+s*height, my+dir*headW);
            cairo_curve_to(cr, ex+s*(height*0.6f), my+dir*headW,
                ex+s*neckH, my+dir*neckW, ex, my+dir*neckW);
            cairo_line_to(cr, x1, y1);
        }
    };

    float pad = std::max(tabW, tabH) + 4.f;
    float ox = pad, oy = pad;
    cairo_move_to(cr, ox, oy);
    drawEdge(ox,   oy,   ox+w, oy,   top,                        true);
    drawEdge(ox+w, oy,   ox+w, oy+h, right,                      false);
    drawEdge(ox+w, oy+h, ox,   oy+h, static_cast<Edge>(-bottom), true);
    drawEdge(ox,   oy+h, ox,   oy,   static_cast<Edge>(-left),   false);
    cairo_close_path(cr);
}

sf::Texture renderPiece(const sf::Image& srcImage,
                        int col, int row, int cellW, int cellH,
                        Edge top, Edge right, Edge bottom, Edge left)
{
    float tabW = cellW * TAB_RATIO, tabH = cellH * TAB_RATIO;
    float pad  = std::max(tabW, tabH) + 4.f;
    int   texW = static_cast<int>(cellW + 2 * pad + 2);
    int   texH = static_cast<int>(cellH + 2 * pad + 2);

    cairo_surface_t* surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, texW, texH);
    cairo_t* cr = cairo_create(surface);
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);

    buildPuzzlePath(cr, cellW, cellH, top, right, bottom, left, tabW, tabH);
    cairo_clip_preserve(cr);

    int srcX = col * cellW - static_cast<int>(pad);
    int srcY = row * cellH - static_cast<int>(pad);
    int imgW = static_cast<int>(srcImage.getSize().x);
    int imgH = static_cast<int>(srcImage.getSize().y);

    // Convert SFML RGBA to Cairo pre-multiplied BGRA
    std::vector<uint8_t> argbBuf(imgW * imgH * 4);
    const uint8_t* px = srcImage.getPixelsPtr();
    for (int i = 0; i < imgW * imgH; ++i) {
        argbBuf[i * 4 + 0] = px[i * 4 + 2];
        argbBuf[i * 4 + 1] = px[i * 4 + 1];
        argbBuf[i * 4 + 2] = px[i * 4 + 0];
        argbBuf[i * 4 + 3] = px[i * 4 + 3];
    }
    cairo_surface_t* imgSurf = cairo_image_surface_create_for_data(
        argbBuf.data(), CAIRO_FORMAT_ARGB32, imgW, imgH, imgW * 4);
    cairo_set_source_surface(cr, imgSurf, -srcX, -srcY);
    cairo_paint(cr);

    // Piece outline
    buildPuzzlePath(cr, cellW, cellH, top, right, bottom, left, tabW, tabH);
    cairo_set_source_rgba(cr, 0, 0, 0, 0.35);
    cairo_set_line_width(cr, OUTLINE_WIDTH);
    cairo_stroke(cr);
    cairo_surface_flush(surface);

    // Convert Cairo BGRA back to SFML RGBA
    unsigned char* bits = cairo_image_surface_get_data(surface);
    sf::Image sfImg;
    sfImg.create(texW, texH);
    for (int y = 0; y < texH; ++y)
        for (int x = 0; x < texW; ++x) {
            int idx = (y * texW + x) * 4;
            sfImg.setPixel(x, y, sf::Color(bits[idx + 2], bits[idx + 1],
                                            bits[idx + 0], bits[idx + 3]));
        }

    sf::Texture tex;
    tex.loadFromImage(sfImg);
    tex.setSmooth(true);
    cairo_surface_destroy(imgSurf);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return tex;
}
