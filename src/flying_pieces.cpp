/**
 * @file flying_pieces.cpp
 * @brief Flying piece animation implementation.
 */

#include "flying_pieces.h"
#include "helpers.h"

#include <algorithm>
#include <cstdlib>

void initFlying(std::vector<FlyingPiece>& fp, int count,
                int winW, int winH, int numTex) {
    fp.resize(count);
    for (auto& f : fp) {
        f.x        = randf(-100.f, static_cast<float>(winW + 100));
        f.y        = randf(-100.f, static_cast<float>(winH + 100));
        f.vx       = randf(-60.f, 60.f);
        f.vy       = randf(-60.f, 60.f);
        f.rot      = randf(0.f, 360.f);
        f.rotSpeed = randf(-90.f, 90.f);
        f.scale    = randf(0.3f, 0.8f);
        f.texIdx   = std::rand() % std::max(1, numTex);
        f.alpha    = static_cast<uint8_t>(randf(80.f, 200.f));
    }
}

void updateFlying(std::vector<FlyingPiece>& fp, float dt, int W, int H) {
    for (auto& f : fp) {
        f.x   += f.vx * dt;
        f.y   += f.vy * dt;
        f.rot += f.rotSpeed * dt;
        if (f.x < -150.f)   f.x = static_cast<float>(W + 100);
        if (f.x > W + 150.f) f.x = -100.f;
        if (f.y < -150.f)   f.y = static_cast<float>(H + 100);
        if (f.y > H + 150.f) f.y = -100.f;
    }
}

void drawFlying(sf::RenderWindow& window,
                const std::vector<FlyingPiece>& fp,
                const std::vector<Piece>& pieces) {
    for (const auto& f : fp) {
        if (f.texIdx >= static_cast<int>(pieces.size())) continue;
        const Piece& p = pieces[f.texIdx];
        sf::Sprite s(p.texture);
        s.setOrigin(static_cast<float>(p.texture.getSize().x) / 2.f,
                    static_cast<float>(p.texture.getSize().y) / 2.f);
        s.setPosition(f.x, f.y);
        s.setRotation(f.rot);
        s.setScale(f.scale, f.scale);
        s.setColor(sf::Color(255, 255, 255, f.alpha));
        window.draw(s);
    }
}
