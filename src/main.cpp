/**
 * @file main.cpp
 * @brief Jigsaw Puzzle game entry point and main loop.
 *
 * Implements the game state machine:
 * Intro -> Name/Difficulty -> Playing -> Win Screen -> Scores
 *
 * All logic is delegated to specialized modules; this file only
 * handles the event loop, state transitions, and per-state rendering.
 */

#include "constants.h"
#include "types.h"
#include "embedded_resources.h"
#include "image_scanner.h"
#include "puzzle_generator.h"
#include "helpers.h"
#include "scores.h"
#include "flying_pieces.h"
#include "ui.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    /* ── scan image directories ─────────────────────────────────────── */
    std::vector<std::string> kidsImages  = scanImages("RES/img_kids");
    std::vector<std::string> adultImages = scanImages("RES/img_adults");

    if (kidsImages.empty() && adultImages.empty()) {
        std::cerr << "No embedded images found\n";
        return 1;
    }

    /** @brief Load an sf::Image from embedded resources by key. */
    auto loadImageRes = [](sf::Image& img, const std::string& key) -> bool {
        const EmbeddedResource* r = getEmbeddedResource(key);
        if (!r) return false;
        return img.loadFromMemory(r->data, r->size);
    };

    /** @brief Load an sf::SoundBuffer from embedded resources by key. */
    auto loadSoundRes = [](sf::SoundBuffer& buf, const std::string& key) -> bool {
        const EmbeddedResource* r = getEmbeddedResource(key);
        if (!r) return false;
        return buf.loadFromMemory(r->data, r->size);
    };

    /* ── load a default image for intro ─────────────────────────────── */
    sf::Image srcImage;
    {
        const std::string& first = kidsImages.empty() ? adultImages[0] : kidsImages[0];
        if (!loadImageRes(srcImage, first)) {
            std::cerr << "Cannot load " << first << "\n";
            return 1;
        }
    }
    int imgW = static_cast<int>(srcImage.getSize().x);
    int imgH = static_cast<int>(srcImage.getSize().y);

    /* ── load sounds ────────────────────────────────────────────────── */
    sf::SoundBuffer bufSnap, bufIncorrect, bufSuccess;
    bool hasSnap      = loadSoundRes(bufSnap,      "sounds/Gnoop.wav");
    bool hasIncorrect = loadSoundRes(bufIncorrect,  "sounds/INCORREC.WAV");
    bool hasSuccess   = loadSoundRes(bufSuccess,    "sounds/SUCCESS.WAV");

    sf::Sound sndSnap, sndIncorrect, sndSuccess;
    if (hasSnap)      sndSnap.setBuffer(bufSnap);
    if (hasIncorrect) sndIncorrect.setBuffer(bufIncorrect);
    if (hasSuccess)   sndSuccess.setBuffer(bufSuccess);

    /* ── fonts ──────────────────────────────────────────────────────── */
    sf::Font font, fontBold;
    bool hasFont = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")
        || font.loadFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf")
        || font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    if (!fontBold.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")
        && !fontBold.loadFromFile("/usr/share/fonts/TTF/DejaVuSans-Bold.ttf")
        && !fontBold.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"))
        fontBold = font;

    /* ── window ─────────────────────────────────────────────────────── */
    int initW = imgW + BOARD_PADDING * 2 + SIDEBAR_W;
    int initH = std::max(imgH + BOARD_PADDING * 2, 500);

    sf::RenderWindow window(sf::VideoMode(initW, initH), "Jigsaw Puzzle",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    /* ── generate initial pieces for intro flying ───────────────────── */
    std::vector<Piece> pieces;
    std::vector<int>   drawOrder;
    int   cellW = 0, cellH = 0;
    float pad = 0.f;
    generatePuzzle(srcImage, 4, 3, pieces, drawOrder, cellW, cellH, pad);

    /* ── game state ─────────────────────────────────────────────────── */
    GameState  state     = STATE_INTRO;
    Difficulty difficulty = DIFF_EASY;
    int  selectedDiff    = 0;
    bool adultMode       = false;
    bool soundEnabled    = false;

    sf::Clock introClock, cursorBlink, frameClock, winClock;

    std::vector<FlyingPiece> introFlying, winFlying;
    initFlying(introFlying, NUM_FLYING, initW, initH,
               static_cast<int>(pieces.size()));

    std::string playerName;
    bool nameCursorVisible = true;

    int  moveCount = 0, placedCount = 0, finalTime = 0;
    bool gameWon = false, scoreSaved = false;
    auto startTime = std::chrono::steady_clock::now();
    int  dragIdx = -1;
    float dragOX = 0.f, dragOY = 0.f;

    std::vector<ScoreEntry> scores;
    int scoreTabFilter = 0;

    int winW = initW, winH = initH;
    int boardW = winW - SIDEBAR_W;

    sf::Vector2f mousePos;
    bool mouseClicked = false;

    /** @brief Convenience: create an sf::Text positioned at (x,y). */
    auto makeText = [&](const std::string& str, float x, float y, int sz,
                        sf::Color col = sf::Color::White) {
        sf::Text t;
        if (hasFont) t.setFont(font);
        t.setString(str);
        t.setCharacterSize(sz);
        t.setFillColor(col);
        t.setPosition(x, y);
        return t;
    };

    /* ═══════════════════════════════════════════════════════════════ */
    /*                         MAIN LOOP                              */
    /* ═══════════════════════════════════════════════════════════════ */

    while (window.isOpen()) {
        float dt = frameClock.restart().asSeconds();
        mouseClicked = false;

        /* ── event handling ─────────────────────────────────────────── */
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();

            if (ev.type == sf::Event::MouseMoved)
                mousePos = {static_cast<float>(ev.mouseMove.x),
                            static_cast<float>(ev.mouseMove.y)};

            /* ── INTRO events ──────────────────────────────────────── */
            if (state == STATE_INTRO) {
                if (ev.type == sf::Event::KeyPressed ||
                    ev.type == sf::Event::MouseButtonPressed) {
                    state = STATE_NAME_INPUT;
                    cursorBlink.restart();
                }
            }

            /* ── NAME INPUT events ─────────────────────────────────── */
            else if (state == STATE_NAME_INPUT) {
                if (ev.type == sf::Event::MouseButtonPressed)
                    mouseClicked = true;

                if (ev.type == sf::Event::TextEntered) {
                    uint32_t ch = ev.text.unicode;
                    if (ch == '\b' || ch == 127) {
                        if (!playerName.empty()) playerName.pop_back();
                    } else if (ch == '\r' || ch == '\n') {
                        if (!playerName.empty()) {
                            adultMode = containsCheatCode(playerName);
                            if (adultMode)
                                playerName = stripCheatCode(playerName);
                            if (playerName.empty())
                                playerName = "Player";

                            const auto& pool = (adultMode && !adultImages.empty())
                                               ? adultImages : kidsImages;
                            std::string chosenFile = pool[std::rand() % pool.size()];
                            if (!loadImageRes(srcImage, chosenFile)) {
                                std::cerr << "Cannot load " << chosenFile << "\n";
                                continue;
                            }
                            imgW = static_cast<int>(srcImage.getSize().x);
                            imgH = static_cast<int>(srcImage.getSize().y);

                            difficulty = static_cast<Difficulty>(selectedDiff);
                            const DiffInfo& di = DIFFICULTIES[difficulty];

                            generatePuzzle(srcImage, di.cols, di.rows,
                                           pieces, drawOrder, cellW, cellH, pad);

                            boardW = imgW + BOARD_PADDING * 2;
                            winW = boardW + SIDEBAR_W;
                            winH = std::max(imgH + BOARD_PADDING * 2, 500);
                            window.setSize(sf::Vector2u(winW, winH));
                            window.setView(sf::View(sf::FloatRect(
                                0, 0, static_cast<float>(winW),
                                static_cast<float>(winH))));

                            for (auto& p : pieces) {
                                p.x = randf(20.f, winW - cellW - 20.f);
                                p.y = randf(20.f, winH - cellH - 20.f);
                                p.sprite.setPosition(p.x - pad, p.y - pad);
                            }
                            moveCount = 0; placedCount = 0;
                            gameWon = false; scoreSaved = false;
                            startTime = std::chrono::steady_clock::now();
                            state = STATE_PLAYING;
                        }
                    } else if (ch >= 32 && ch < 127 &&
                               static_cast<int>(playerName.size()) < MAX_NAME_LEN) {
                        playerName += static_cast<char>(ch);
                    }
                }
            }

            /* ── PLAYING events ────────────────────────────────────── */
            else if (state == STATE_PLAYING) {
                if (ev.type == sf::Event::MouseButtonPressed &&
                    ev.mouseButton.button == sf::Mouse::Left) {
                    float mx = static_cast<float>(ev.mouseButton.x);
                    float my = static_cast<float>(ev.mouseButton.y);

                    sf::FloatRect scatterRect(boardW + 20.f, winH - 60.f,
                                              SIDEBAR_W - 40.f, 36.f);
                    if (scatterRect.contains(mx, my)) {
                        for (auto& p : pieces) {
                            if (p.placed) continue;
                            p.x = randf(20.f, winW - cellW - 20.f);
                            p.y = randf(20.f, winH - cellH - 20.f);
                            p.sprite.setPosition(p.x - pad, p.y - pad);
                        }
                        continue;
                    }

                    for (int i = static_cast<int>(drawOrder.size()) - 1;
                         i >= 0; --i) {
                        int pi = drawOrder[i];
                        Piece& p = pieces[pi];
                        if (p.placed) continue;
                        sf::FloatRect bounds(p.x - pad, p.y - pad,
                            cellW + 2.f * pad, cellH + 2.f * pad);
                        if (bounds.contains(mx, my)) {
                            dragIdx = pi;
                            dragOX = mx - p.x;
                            dragOY = my - p.y;
                            p.dragging = true;
                            drawOrder.erase(drawOrder.begin() + i);
                            drawOrder.push_back(pi);
                            break;
                        }
                    }
                }

                if (ev.type == sf::Event::MouseButtonReleased &&
                    ev.mouseButton.button == sf::Mouse::Left && dragIdx >= 0) {
                    Piece& p = pieces[dragIdx];
                    p.dragging = false;
                    float dx = p.x - p.homeX, dy = p.y - p.homeY;

                    if (std::sqrt(dx * dx + dy * dy) < SNAP_DIST) {
                        p.x = p.homeX;
                        p.y = p.homeY;
                        p.sprite.setPosition(p.x - pad, p.y - pad);
                        if (!p.placed) {
                            p.placed = true;
                            ++placedCount;
                            ++moveCount;

                            if (placedCount == static_cast<int>(pieces.size())) {
                                gameWon = true;
                                finalTime = static_cast<int>(
                                    std::chrono::duration_cast<std::chrono::seconds>(
                                        std::chrono::steady_clock::now()
                                        - startTime).count());
                                if (hasSuccess && soundEnabled) sndSuccess.play();
                                state = STATE_WIN_SCREEN;
                                winClock.restart();
                                initFlying(winFlying, NUM_FLYING + 6,
                                           winW, winH,
                                           static_cast<int>(pieces.size()));
                            } else {
                                if (hasSnap && soundEnabled) sndSnap.play();
                            }
                        }
                        auto it = std::find(drawOrder.begin(),
                                            drawOrder.end(), dragIdx);
                        if (it != drawOrder.end()) {
                            drawOrder.erase(it);
                            drawOrder.insert(drawOrder.begin(), dragIdx);
                        }
                    } else {
                        ++moveCount;
                        if (hasIncorrect && soundEnabled) sndIncorrect.play();
                    }
                    dragIdx = -1;
                }

                if (ev.type == sf::Event::MouseMoved && dragIdx >= 0) {
                    Piece& p = pieces[dragIdx];
                    p.x = static_cast<float>(ev.mouseMove.x) - dragOX;
                    p.y = static_cast<float>(ev.mouseMove.y) - dragOY;
                    p.sprite.setPosition(p.x - pad, p.y - pad);
                }
            }

            /* ── WIN / SCORES events ───────────────────────────────── */
            else if (state == STATE_WIN_SCREEN || state == STATE_SCORES) {
                if (ev.type == sf::Event::MouseButtonPressed)
                    mouseClicked = true;
            }
        }

        /* ═══════════════════════════════════════════════════════════ */
        /*                         RENDERING                          */
        /* ═══════════════════════════════════════════════════════════ */

        window.clear(sf::Color(32, 32, 38));
        float cx = winW / 2.f, cy = winH / 2.f;

        /* ── INTRO ─────────────────────────────────────────────────── */
        if (state == STATE_INTRO) {
            float t = introClock.getElapsedTime().asSeconds();
            updateFlying(introFlying, dt, winW, winH);
            drawFlying(window, introFlying, pieces);

            float a = std::min(1.f, t / 1.5f);
            uint8_t alpha = static_cast<uint8_t>(a * 255);
            if (hasFont) {
                drawCentered(window, fontBold, "JIGSAW PUZZLE",
                    cy - 60.f, 42, sf::Color(230, 230, 255, alpha), cx);
                drawCentered(window, font, "Assemble the picture from pieces",
                    cy, 18, sf::Color(180, 180, 200, alpha), cx);
                float pulse = 0.5f + 0.5f * std::sin(t * 3.f);
                drawCentered(window, font, "Press any key to start",
                    cy + 60.f, 15, sf::Color(150, 200, 255,
                        static_cast<uint8_t>(pulse * a * 255)), cx);
            }
            if (t > INTRO_DURATION) {
                state = STATE_NAME_INPUT;
                cursorBlink.restart();
            }
        }

        /* ── NAME INPUT + DIFFICULTY ───────────────────────────────── */
        else if (state == STATE_NAME_INPUT) {
            updateFlying(introFlying, dt, winW, winH);
            drawFlying(window, introFlying, pieces);

            sf::RectangleShape overlay(
                {static_cast<float>(winW), static_cast<float>(winH)});
            overlay.setFillColor(sf::Color(18, 18, 24, 220));
            window.draw(overlay);

            if (hasFont) {
                drawCentered(window, fontBold, "ENTER YOUR NAME",
                    cy - 130.f, 28, sf::Color(220, 220, 240), cx);

                float boxW = 300.f, boxH = 44.f;
                sf::RectangleShape inputBox({boxW, boxH});
                inputBox.setOrigin(boxW / 2.f, boxH / 2.f);
                inputBox.setPosition(cx, cy - 80.f);
                inputBox.setFillColor(sf::Color(40, 40, 50));
                inputBox.setOutlineColor(sf::Color(80, 140, 220));
                inputBox.setOutlineThickness(2.f);
                window.draw(inputBox);

                if (cursorBlink.getElapsedTime().asSeconds() > 0.5f) {
                    nameCursorVisible = !nameCursorVisible;
                    cursorBlink.restart();
                }
                std::string display = playerName
                                    + (nameCursorVisible ? "|" : "");
                sf::Text inputText;
                inputText.setFont(font);
                inputText.setString(display);
                inputText.setCharacterSize(22);
                inputText.setFillColor(sf::Color::White);
                sf::FloatRect tr = inputText.getLocalBounds();
                inputText.setOrigin(tr.left + tr.width / 2.f,
                                    tr.top + tr.height / 2.f);
                inputText.setPosition(cx, cy - 80.f);
                window.draw(inputText);

                if (containsCheatCode(playerName) && !adultImages.empty()) {
                    drawCentered(window, font, "* Adult mode activated *",
                        cy - 50.f, 11, sf::Color(255, 80, 80), cx);
                }

                drawCentered(window, fontBold, "SELECT DIFFICULTY",
                    cy - 30.f, 22, sf::Color(200, 200, 220), cx);

                float btnY = cy + 20.f;
                float spacing = 160.f;
                for (int d = 0; d < 3; ++d) {
                    const DiffInfo& di = DIFFICULTIES[d];
                    float bx = cx + (d - 1) * spacing;
                    bool selected = (d == selectedDiff);

                    sf::RectangleShape btn({140.f, 60.f});
                    btn.setOrigin(70.f, 30.f);
                    btn.setPosition(bx, btnY);
                    bool hovered = btn.getGlobalBounds().contains(mousePos);
                    btn.setFillColor(hovered && !selected
                        ? sf::Color(65, 65, 80)
                        : (selected ? di.color : sf::Color(50, 50, 65)));
                    btn.setOutlineColor(selected ? sf::Color::White
                                                 : sf::Color(90, 90, 110));
                    btn.setOutlineThickness(selected ? 2.f : 1.f);
                    window.draw(btn);

                    drawCentered(window, fontBold, di.label,
                        btnY - 22.f, 16, sf::Color::White, bx);
                    drawCentered(window, font,
                        std::to_string(di.cols * di.rows) + " pieces",
                        btnY + 2.f, 12, sf::Color(180, 180, 200), bx);

                    if (hovered && mouseClicked)
                        selectedDiff = d;
                }

                // Sound checkbox
                float cbY = cy + 65.f;
                float cbX = cx - 75.f;
                float cbSize = 18.f;

                sf::RectangleShape cbBox({cbSize, cbSize});
                cbBox.setPosition(cbX, cbY);
                cbBox.setFillColor(soundEnabled ? sf::Color(50, 160, 90)
                                                : sf::Color(45, 45, 55));
                cbBox.setOutlineColor(soundEnabled ? sf::Color(80, 200, 120)
                                                   : sf::Color(90, 90, 110));
                cbBox.setOutlineThickness(1.5f);
                window.draw(cbBox);

                if (soundEnabled) {
                    // Draw checkmark
                    sf::Text check;
                    check.setFont(font);
                    check.setString("x");
                    check.setCharacterSize(14);
                    check.setFillColor(sf::Color::White);
                    check.setPosition(cbX + 3.f, cbY - 1.f);
                    window.draw(check);
                }

                drawCentered(window, font, "Sound effects",
                    cbY + 1.f, 14, sf::Color(180, 180, 200), cbX + cbSize + 60.f);

                // Click handling for checkbox
                if (mouseClicked) {
                    sf::FloatRect cbRect(cbX, cbY, cbSize + 120.f, cbSize);
                    if (cbRect.contains(mousePos))
                        soundEnabled = !soundEnabled;
                }

                drawCentered(window, font, "Press ENTER to begin",
                    cy + 95.f, 14, sf::Color(140, 140, 170), cx);
            }
        }

        /* ── PLAYING ───────────────────────────────────────────────── */
        else if (state == STATE_PLAYING) {
            int elapsed = static_cast<int>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - startTime).count());

            sf::RectangleShape boardBg({imgW + 2.f, imgH + 2.f});
            boardBg.setPosition(BOARD_PADDING - 1.f, BOARD_PADDING - 1.f);
            boardBg.setFillColor(sf::Color(255, 255, 255, 18));
            boardBg.setOutlineColor(sf::Color(255, 255, 255, 40));
            boardBg.setOutlineThickness(1.f);
            window.draw(boardBg);

            const DiffInfo& di = DIFFICULTIES[difficulty];
            for (int c = 1; c < di.cols; ++c) {
                sf::RectangleShape l({1.f, static_cast<float>(imgH)});
                l.setPosition(BOARD_PADDING + c * cellW, BOARD_PADDING);
                l.setFillColor(sf::Color(255, 255, 255, 22));
                window.draw(l);
            }
            for (int r = 1; r < di.rows; ++r) {
                sf::RectangleShape l({static_cast<float>(imgW), 1.f});
                l.setPosition(BOARD_PADDING, BOARD_PADDING + r * cellH);
                l.setFillColor(sf::Color(255, 255, 255, 22));
                window.draw(l);
            }

            for (int pi : drawOrder) {
                const Piece& p = pieces[pi];
                if (!p.placed) {
                    sf::Sprite sh(p.sprite);
                    sh.setColor(sf::Color(0, 0, 0, 60));
                    sh.move(SHADOW_OFFSET, SHADOW_OFFSET);
                    window.draw(sh);
                }
                window.draw(p.sprite);
            }

            // Sidebar
            sf::RectangleShape sidebar(
                {static_cast<float>(SIDEBAR_W), static_cast<float>(winH)});
            sidebar.setPosition(static_cast<float>(boardW), 0.f);
            sidebar.setFillColor(sf::Color(22, 22, 28));
            window.draw(sidebar);

            sf::RectangleShape sline({1.f, static_cast<float>(winH)});
            sline.setPosition(static_cast<float>(boardW), 0.f);
            sline.setFillColor(sf::Color(60, 60, 70));
            window.draw(sline);

            sf::Texture thumbTex;
            thumbTex.loadFromImage(srcImage);
            thumbTex.setSmooth(true);
            sf::Sprite thumb(thumbTex);
            float ts = static_cast<float>(SIDEBAR_W - 20) / imgW;
            thumb.setScale(ts, ts);
            thumb.setPosition(boardW + 10.f, 80.f);

            if (hasFont)
                window.draw(makeText("PREVIEW", boardW + 10.f, 14.f, 13,
                                     sf::Color(180, 180, 200)));
            window.draw(thumb);

            float infoY = 80.f + imgH * ts + 20.f;
            if (hasFont) {
                window.draw(makeText(playerName, boardW + 20.f, 36.f, 14,
                                     sf::Color(100, 180, 255)));
                window.draw(makeText(diffName(difficulty),
                    boardW + 20.f, 54.f, 12, DIFFICULTIES[difficulty].color));

                window.draw(makeText("Time", boardW + 20.f, infoY, 13,
                                     sf::Color(140, 140, 160)));
                window.draw(makeText(formatTime(elapsed),
                    boardW + 20.f, infoY + 18.f, 22));

                window.draw(makeText("Moves", boardW + 20.f, infoY + 58.f,
                    13, sf::Color(140, 140, 160)));
                window.draw(makeText(std::to_string(moveCount),
                    boardW + 20.f, infoY + 76.f, 22));

                window.draw(makeText("Placed", boardW + 20.f, infoY + 116.f,
                    13, sf::Color(140, 140, 160)));
                window.draw(makeText(
                    std::to_string(placedCount) + " / "
                    + std::to_string(pieces.size()),
                    boardW + 20.f, infoY + 134.f, 22));

                sf::RectangleShape scBtn({SIDEBAR_W - 40.f, 36.f});
                scBtn.setPosition(boardW + 20.f, winH - 60.f);
                scBtn.setFillColor(sf::Color(50, 120, 200));
                scBtn.setOutlineColor(sf::Color(80, 150, 230));
                scBtn.setOutlineThickness(1.f);
                window.draw(scBtn);
                window.draw(makeText("Scatter",
                    boardW + 60.f, winH - 54.f, 15));
            }
        }

        /* ── WIN SCREEN ────────────────────────────────────────────── */
        else if (state == STATE_WIN_SCREEN) {
            float t = winClock.getElapsedTime().asSeconds();
            updateFlying(winFlying, dt, winW, winH);

            sf::RectangleShape boardBg({imgW + 2.f, imgH + 2.f});
            boardBg.setPosition(BOARD_PADDING - 1.f, BOARD_PADDING - 1.f);
            boardBg.setFillColor(sf::Color(255, 255, 255, 18));
            window.draw(boardBg);
            for (int pi : drawOrder)
                window.draw(pieces[pi].sprite);

            drawFlying(window, winFlying, pieces);

            float da = std::min(180.f, t * 120.f);
            sf::RectangleShape wo(
                {static_cast<float>(winW), static_cast<float>(winH)});
            wo.setFillColor(sf::Color(12, 12, 20, static_cast<uint8_t>(da)));
            window.draw(wo);

            if (hasFont) {
                float sc = 1.f + 0.04f * std::sin(t * 2.5f);
                sf::Text title;
                title.setFont(fontBold);
                title.setString("PUZZLE COMPLETE!");
                title.setCharacterSize(38);
                title.setFillColor(sf::Color(100, 255, 140));
                sf::FloatRect tr2 = title.getLocalBounds();
                title.setOrigin(tr2.left + tr2.width / 2.f,
                                tr2.top + tr2.height / 2.f);
                title.setPosition(cx, cy - 90.f);
                title.setScale(sc, sc);
                window.draw(title);

                drawCentered(window, font,
                    "Congratulations, " + playerName + "!",
                    cy - 40.f, 20, sf::Color(220, 220, 240), cx);

                drawCentered(window, font,
                    std::string(diffName(difficulty)) + "   Time: "
                    + formatTime(finalTime) + "   Moves: "
                    + std::to_string(moveCount),
                    cy + 5.f, 18, sf::Color(200, 200, 220), cx);

                if (drawButton(window, font, "View Scores",
                    cx, cy + 70.f, 180.f, 42.f, mousePos, mouseClicked)) {
                    if (!scoreSaved) {
                        saveScore({playerName, finalTime,
                                   moveCount, difficulty});
                        scoreSaved = true;
                    }
                    scores = loadScores();
                    scoreTabFilter = difficulty;
                    state = STATE_SCORES;
                }
            }
        }

        /* ── SCORES ────────────────────────────────────────────────── */
        else if (state == STATE_SCORES) {
            updateFlying(winFlying, dt, winW, winH);
            drawFlying(window, winFlying, pieces);
            sf::RectangleShape overlay(
                {static_cast<float>(winW), static_cast<float>(winH)});
            overlay.setFillColor(sf::Color(18, 18, 24, 220));
            window.draw(overlay);

            if (hasFont) {
                drawCentered(window, fontBold, "HIGH SCORES",
                    20.f, 32, sf::Color(255, 215, 80), cx);

                float tabY = 70.f;
                for (int d = 0; d < 3; ++d) {
                    const DiffInfo& di = DIFFICULTIES[d];
                    float tx = cx + (d - 1) * 140.f;
                    bool active = (d == scoreTabFilter);

                    sf::RectangleShape tab({120.f, 32.f});
                    tab.setOrigin(60.f, 16.f);
                    tab.setPosition(tx, tabY);
                    bool hovered = tab.getGlobalBounds().contains(mousePos);
                    tab.setFillColor(hovered && !active
                        ? sf::Color(60, 60, 75)
                        : (active ? di.color : sf::Color(45, 45, 55)));
                    tab.setOutlineColor(active ? sf::Color::White
                                               : sf::Color(80, 80, 100));
                    tab.setOutlineThickness(active ? 2.f : 1.f);
                    window.draw(tab);

                    drawCentered(window, active ? fontBold : font,
                        di.label, tabY - 10.f, 14, sf::Color::White, tx);

                    if (hovered && mouseClicked)
                        scoreTabFilter = d;
                }

                std::vector<ScoreEntry> filtered;
                for (auto& s : scores)
                    if (s.difficulty == scoreTabFilter)
                        filtered.push_back(s);
                std::sort(filtered.begin(), filtered.end(),
                    [](const ScoreEntry& a, const ScoreEntry& b) {
                        return a.timeSec < b.timeSec;
                    });

                float tableX = cx - 210.f;
                float rowY   = 110.f;
                float colName  = tableX;
                float colTime  = tableX + 210.f;
                float colMoves = tableX + 330.f;

                sf::RectangleShape hl({420.f, 1.f});
                hl.setPosition(tableX - 30.f, rowY + 28.f);
                hl.setFillColor(sf::Color(100, 100, 130));
                window.draw(hl);

                auto hc = sf::Color(160, 160, 190);
                window.draw(makeText("#",     tableX - 30.f, rowY, 14, hc));
                window.draw(makeText("Name",  colName,       rowY, 14, hc));
                window.draw(makeText("Time",  colTime,       rowY, 14, hc));
                window.draw(makeText("Moves", colMoves,      rowY, 14, hc));

                rowY += 36.f;
                int maxShow = std::min(
                    static_cast<int>(filtered.size()), 10);
                for (int i = 0; i < maxShow; ++i) {
                    const ScoreEntry& s = filtered[i];
                    float y = rowY + i * 30.f;
                    bool cur = (s.name == playerName
                                && s.timeSec == finalTime
                                && s.moves == moveCount
                                && s.difficulty == difficulty);
                    sf::Color rc = cur ? sf::Color(100, 255, 160)
                                       : sf::Color(210, 210, 230);
                    sf::Color rk = rc;
                    if (i == 0) rk = sf::Color(255, 215, 80);
                    if (i == 1) rk = sf::Color(200, 200, 210);
                    if (i == 2) rk = sf::Color(205, 127, 50);

                    window.draw(makeText(
                        std::to_string(i + 1) + ".",
                        tableX - 30.f, y, 15, rk));
                    window.draw(makeText(s.name, colName, y, 15, rc));
                    window.draw(makeText(
                        formatTime(s.timeSec), colTime, y, 15, rc));
                    window.draw(makeText(
                        std::to_string(s.moves), colMoves, y, 15, rc));
                }

                if (filtered.empty())
                    drawCentered(window, font,
                        "No scores yet for this level",
                        rowY + 40.f, 16, sf::Color(120, 120, 150), cx);

                float btnYpos = static_cast<float>(winH - 50);
                if (drawButton(window, font, "Play Again",
                    cx - 100.f, btnYpos, 160.f, 40.f,
                    mousePos, mouseClicked)) {
                    state = STATE_NAME_INPUT;
                    playerName.clear();
                    moveCount = 0; placedCount = 0;
                    gameWon = false; scoreSaved = false;
                    dragIdx = -1; adultMode = false;

                    if (!kidsImages.empty())
                        loadImageRes(srcImage, kidsImages[0]);
                    imgW = static_cast<int>(srcImage.getSize().x);
                    imgH = static_cast<int>(srcImage.getSize().y);
                    generatePuzzle(srcImage, 4, 3, pieces, drawOrder,
                                   cellW, cellH, pad);

                    winW = initW; winH = initH;
                    boardW = winW - SIDEBAR_W;
                    window.setSize(sf::Vector2u(winW, winH));
                    window.setView(sf::View(sf::FloatRect(
                        0, 0, static_cast<float>(winW),
                        static_cast<float>(winH))));
                    initFlying(introFlying, NUM_FLYING, winW, winH,
                               static_cast<int>(pieces.size()));
                    cursorBlink.restart();
                }

                if (drawButton(window, font, "Quit",
                    cx + 100.f, btnYpos, 160.f, 40.f,
                    mousePos, mouseClicked))
                    window.close();
            }
        }

        window.display();
    }
    return 0;
}
