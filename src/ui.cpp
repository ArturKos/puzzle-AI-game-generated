/**
 * @file ui.cpp
 * @brief UI drawing helper implementations.
 */

#include "ui.h"

void drawCentered(sf::RenderWindow& window, const sf::Font& font,
                  const std::string& str, float y, int size,
                  sf::Color col, float centerX) {
    sf::Text t;
    t.setFont(font);
    t.setString(str);
    t.setCharacterSize(size);
    t.setFillColor(col);
    sf::FloatRect r = t.getLocalBounds();
    t.setOrigin(r.left + r.width / 2.f, r.top);
    t.setPosition(centerX, y);
    window.draw(t);
}

bool drawButton(sf::RenderWindow& window, const sf::Font& font,
                const std::string& label, float cx, float cy,
                float bw, float bh, sf::Vector2f mp, bool clicked,
                sf::Color base, sf::Color hover, sf::Color outline) {
    sf::RectangleShape btn(sf::Vector2f(bw, bh));
    btn.setOrigin(bw / 2.f, bh / 2.f);
    btn.setPosition(cx, cy);
    bool hovered = btn.getGlobalBounds().contains(mp);
    btn.setFillColor(hovered ? hover : base);
    btn.setOutlineColor(outline);
    btn.setOutlineThickness(1.f);
    window.draw(btn);

    sf::Text t;
    t.setFont(font);
    t.setString(label);
    t.setCharacterSize(15);
    t.setFillColor(sf::Color::White);
    sf::FloatRect r = t.getLocalBounds();
    t.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
    t.setPosition(cx, cy);
    window.draw(t);

    return hovered && clicked;
}
