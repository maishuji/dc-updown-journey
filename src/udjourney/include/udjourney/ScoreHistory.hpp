// Copyright 2025 Quentin Cartier
#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_SCOREHISTORY_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_SCOREHISTORY_HPP_

#include <string>
#include <vector>

template <typename T> class ScoreHistory {
 public:
    ScoreHistory() = default;
    inline void add_score(T score) noexcept { scores_.push_back(score); }
    [[nodiscard]] inline T get_last_score() const noexcept {
        return scores_.empty() ? 0 : scores_.back();
    }
    [[nodiscard]] inline size_t get_size() const noexcept {
        return scores_.size();
    }
    [[nodiscard]] inline T get_score(size_t idx) const noexcept {
        return (idx < scores_.size()) ? scores_[idx] : 0;
    }
    [[nodiscard]] std::vector<T> get_scores() const noexcept { return scores_; }

 private:
    std::vector<T> scores_;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_SCOREHISTORY_HPP_
