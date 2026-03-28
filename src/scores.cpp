/**
 * @file scores.cpp
 * @brief Score persistence implementation.
 */

#include "scores.h"
#include "constants.h"

#include <fstream>
#include <string>

std::vector<ScoreEntry> loadScores() {
    std::vector<ScoreEntry> scores;
    std::ifstream f(SCORES_FILE);
    if (!f.is_open()) return scores;

    std::string line;
    while (std::getline(f, line)) {
        size_t p1 = line.find('|');
        size_t p2 = line.find('|', p1 + 1);
        size_t p3 = line.find('|', p2 + 1);
        if (p1 == std::string::npos || p2 == std::string::npos) continue;

        ScoreEntry e;
        e.name    = line.substr(0, p1);
        e.timeSec = std::stoi(line.substr(p1 + 1, p2 - p1 - 1));
        e.moves   = std::stoi(line.substr(p2 + 1,
                        (p3 != std::string::npos ? p3 - p2 - 1 : std::string::npos)));
        e.difficulty = (p3 != std::string::npos) ? std::stoi(line.substr(p3 + 1)) : 0;
        scores.push_back(e);
    }
    return scores;
}

void saveScore(const ScoreEntry& e) {
    std::ofstream f(SCORES_FILE, std::ios::app);
    f << e.name << "|" << e.timeSec << "|" << e.moves << "|" << e.difficulty << "\n";
}
