/**
 * @file ui.h
 * @brief Reusable UI drawing helpers: centered text, buttons.
 */

#pragma once

#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief Draw horizontally centered text at a given Y position.
 *
 * @param window   Target render window.
 * @param font     Font to use.
 * @param str      Text string to display.
 * @param y        Y position of the text baseline.
 * @param size     Character size in pixels.
 * @param col      Text fill color.
 * @param centerX  X coordinate to center the text around.
 */
void drawCentered(sf::RenderWindow& window, const sf::Font& font,
                  const std::string& str, float y, int size,
                  sf::Color col, float centerX);

/**
 * @brief Draw an interactive button and report clicks.
 *
 * Renders a rounded rectangle with hover highlighting and centered label.
 * Returns true if the button was clicked this frame (hovered + clicked).
 *
 * @param window   Target render window.
 * @param font     Font for the label text.
 * @param label    Button text.
 * @param cx       Center X of the button.
 * @param cy       Center Y of the button.
 * @param bw       Button width.
 * @param bh       Button height.
 * @param mp       Current mouse position.
 * @param clicked  Whether a mouse click occurred this frame.
 * @param base     Normal fill color (default: blue).
 * @param hover    Hover fill color (default: lighter blue).
 * @param outline  Outline color (default: light blue).
 * @return true if the button was clicked.
 */
bool drawButton(sf::RenderWindow& window, const sf::Font& font,
                const std::string& label, float cx, float cy,
                float bw, float bh, sf::Vector2f mp, bool clicked,
                sf::Color base    = sf::Color(50, 110, 190),
                sf::Color hover   = sf::Color(70, 140, 220),
                sf::Color outline = sf::Color(100, 170, 255));
