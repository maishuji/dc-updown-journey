// Copyright 2025 Quentin Cartier
#include "udjourney/hud/ScoreHUD.hpp"

#include <raylib/raylib.h>

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include "udjourney/interfaces/IActor.hpp"

namespace {
// Function definition for extract_number_
std::optional<int16_t> extract_number_(const std::string_view &iStrView) {
    std::string number;
    for (char letter : iStrView) {
        auto is_digit = std::isdigit(letter);
        if (is_digit != 0) {
            number += letter;
        }
    }
    try {
        return std::stoi(number);  // Convert each token to integer
    } catch (const std::invalid_argument &e) {
        std::cerr << "Invalid number: " << number << std::endl;
    } catch (const std::out_of_range &e) {
        std::cerr << "Number out of range: " << number << std::endl;
    }
    return {};
}
}  // namespace

ScoreHUD::ScoreHUD(Vector2 iPosition, IObservable *ioNullableObservable) :
    m_position(iPosition), m_observable_actor(ioNullableObservable) {
    if (ioNullableObservable != nullptr) {
        ioNullableObservable->add_observer(this);
    }
}

/**
 * @brief Destructor for the ScoreHUD.
 *
 * Ensures that this observer is properly unregistered from the observable actor
 * to prevent dangling pointers and potential undefined behavior.
 */
ScoreHUD::~ScoreHUD() {
    if (m_observable_actor != nullptr) {
        m_observable_actor->remove_observer(this);
    }
}

void ScoreHUD::on_notify(const std::string &iEvent) {
    std::stringstream str_stream(iEvent);
    std::string token;
    int mode = 0;
    std::cout << " aaa" << std::endl;

    const int16_t kModeGameOuver = 12;
    const int16_t kModeScoring = 1;
    const int16_t kModeBonus = 2;
    const int16_t kModeDash = 4;

    // Parse event mode
    if (std::getline(str_stream, token, ';')) {
        // First token is the mode
        auto mode_opt = extract_number_(token);
        if (mode_opt.has_value()) {
            mode = mode_opt.value();
        } else {
            std::cerr << "Invalid mode: " << token << std::endl;
            return;
        }
    }  // Split by ';'

    if (mode == kModeScoring) {
        // Parsing scoring event
        std::getline(str_stream, token, ';');
        if (std::optional<int16_t> score_inc_opt = extract_number_(token);
            score_inc_opt.has_value()) {
            m_score += score_inc_opt.value();
        }
    }
}
