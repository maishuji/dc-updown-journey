// Copyright 2025 Quentin Cartier
#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_SCOREHISTORY_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_SCOREHISTORY_HPP_

#include <string>
#include <vector>

class ScoreHistory {
 public:
    ScoreHistory() = default;
    void increment_current_score(int score) noexcept;
    void add_score(int score) noexcept;
    [[nodiscard]] int get_last_score() const noexcept;
    [[nodiscard]] std::vector<int> get_scores() const noexcept;

 private:
    std::vector<int> scores_;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_SCOREHISTORY_HPP_
