// Copyright 2025 Quentin Cartier
#include "udjourney/ScoreHistory.hpp"

#include <vector>

void ScoreHistory::add_score(int score) noexcept {}

std::vector<int> ScoreHistory::get_scores() const noexcept { return scores_; }

int ScoreHistory::get_last_score() const noexcept {
    if (scores_.empty()) {
        return -1;
    }
    return scores_.back();
}

void ScoreHistory::increment_current_score(int score) noexcept {
    if (scores_.empty()) {
        scores_.push_back(score);
    } else {
        scores_.back() += score;
    }
}
