// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include "udjourney/interfaces/IActor.hpp"

/**
 * @brief Unified manager for all game actors
 *
 * Handles update, rendering, and lifecycle management for all actor types
 * (player, monsters, bonuses, platforms, projectiles, etc.)
 *
 * This replaces separate managers for each actor type with a single,
 * generic system based on the IActor interface.
 */
class ActorManager {
 public:
    ActorManager() = default;
    ~ActorManager() = default;

    // Prevent copying
    ActorManager(const ActorManager&) = delete;
    ActorManager& operator=(const ActorManager&) = delete;

    // ============= Actor Lifecycle =============

    /**
     * @brief Add an actor to the manager
     * @return Pointer to the added actor (ownership transferred to manager)
     */
    IActor* add_actor(std::unique_ptr<IActor> actor) {
        if (!actor) return nullptr;
        IActor* ptr = actor.get();
        actors_.push_back(std::move(actor));
        return ptr;
    }

    /**
     * @brief Mark actors for removal (removed at end of update)
     * Actors with state == ActorState::CONSUMED will be removed
     */
    void cleanup_consumed_actors() {
        actors_.erase(std::remove_if(actors_.begin(),
                                     actors_.end(),
                                     [](const std::unique_ptr<IActor>& actor) {
                                         return actor->get_state() ==
                                                ActorState::CONSUMED;
                                     }),
                      actors_.end());
    }

    /**
     * @brief Remove all actors
     */
    void clear_all_actors() { actors_.clear(); }

    // ============= Update & Render =============

    /**
     * @brief Update all actors
     */
    void update_all(float delta) {
        for (auto& actor : actors_) {
            if (actor->get_state() == ActorState::ONGOING) {
                actor->update(delta);
            }
        }
        cleanup_consumed_actors();
    }

    /**
     * @brief Render all actors
     */
    void draw_all() const {
        for (const auto& actor : actors_) {
            if (actor->get_state() == ActorState::ONGOING) {
                actor->draw();
            }
        }
    }

    // ============= Queries =============

    /**
     * @brief Get all actors
     */
    [[nodiscard]] const std::vector<std::unique_ptr<IActor>>& get_all_actors()
        const {
        return actors_;
    }

    /**
     * @brief Get actors by group ID
     * @param group_id Group identifier (0=player, 1=platform, 2=bonus,
     * 3=monster)
     */
    [[nodiscard]] std::vector<IActor*> get_actors_by_group(
        uint8_t group_id) const {
        std::vector<IActor*> result;
        for (const auto& actor : actors_) {
            if (actor->get_group_id() == group_id &&
                actor->get_state() == ActorState::ONGOING) {
                result.push_back(actor.get());
            }
        }
        return result;
    }

    /**
     * @brief Get player actor (group_id == 0)
     */
    [[nodiscard]] IActor* get_player() const {
        for (const auto& actor : actors_) {
            if (actor->get_group_id() == 0 &&
                actor->get_state() == ActorState::ONGOING) {
                return actor.get();
            }
        }
        return nullptr;
    }

    /**
     * @brief Get all monsters (group_id == 3)
     */
    [[nodiscard]] std::vector<IActor*> get_monsters() const {
        return get_actors_by_group(3);
    }

    /**
     * @brief Get all platforms (group_id == 1)
     */
    [[nodiscard]] std::vector<IActor*> get_platforms() const {
        return get_actors_by_group(1);
    }

    /**
     * @brief Get all bonuses (group_id == 2)
     */
    [[nodiscard]] std::vector<IActor*> get_bonuses() const {
        return get_actors_by_group(2);
    }

    /**
     * @brief Count actors by group
     */
    [[nodiscard]] size_t count_by_group(uint8_t group_id) const {
        return std::count_if(actors_.begin(),
                             actors_.end(),
                             [group_id](const std::unique_ptr<IActor>& actor) {
                                 return actor->get_group_id() == group_id &&
                                        actor->get_state() ==
                                            ActorState::ONGOING;
                             });
    }

    /**
     * @brief Total number of active actors
     */
    [[nodiscard]] size_t get_actor_count() const {
        return std::count_if(actors_.begin(),
                             actors_.end(),
                             [](const std::unique_ptr<IActor>& actor) {
                                 return actor->get_state() ==
                                        ActorState::ONGOING;
                             });
    }

 private:
    std::vector<std::unique_ptr<IActor>> actors_;
};
