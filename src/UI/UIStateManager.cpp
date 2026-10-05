#include "UI/UIStateManager.h"

#include "Gameplay/LossManager.h"

namespace LossGauge
{
    UIStateManager* UIStateManager::GetSingleton()
    {
        static UIStateManager singleton;
        return &singleton;
    }

    bool UIStateManager::Update()
    {
        auto* manager = LossManager::GetSingleton();

        if (!manager) {
            return false;
        }

        UIState newState{};

        newState.currentHealth =
            manager->GetCurrentHealth();

        newState.maxHealth =
            manager->GetMaxHealth();

        newState.recoverableHealth =
            manager->GetRecoverableHealth();

        // ========================================
        // Validate raw values
        // ========================================

        if (!std::isfinite(newState.currentHealth) ||
            !std::isfinite(newState.maxHealth) ||
            !std::isfinite(newState.recoverableHealth)) {

            return false;
        }

        if (newState.maxHealth <= kEpsilon) {
            return false;
        }

        // ========================================
        // Calculate normalized UI state
        // ========================================

        newState.currentPct =
            newState.currentHealth /
            newState.maxHealth;

        newState.recoverablePct =
            newState.recoverableHealth /
            newState.maxHealth;

        newState.lossPct =
            1.0f -
            newState.recoverablePct;

        // ========================================
        // Safety clamp
        // ========================================

        newState.currentPct =
            std::clamp(
                newState.currentPct,
                0.0f,
                1.0f);

        newState.recoverablePct =
            std::clamp(
                newState.recoverablePct,
                0.0f,
                1.0f);

        newState.lossPct =
            std::clamp(
                newState.lossPct,
                0.0f,
                1.0f);

        // Current HP should never visually extend
        // beyond Recoverable HP.
        newState.currentPct =
            (std::min)(
                newState.currentPct,
                newState.recoverablePct);

        // ========================================
        // Initial state
        // ========================================

        if (!initialized_) {
            state_ = newState;
            initialized_ = true;

            logs::info(
                "================================");

            logs::info(
                "Loss Gauge UI State");

            logs::info(
                "--------------------------------");

            logs::info(
                "Current HP:     {:.2f}",
                state_.currentHealth);

            logs::info(
                "Max HP:         {:.2f}",
                state_.maxHealth);

            logs::info(
                "Recoverable HP: {:.2f}",
                state_.recoverableHealth);

            logs::info(
                "Current:        {:.2f}%",
                state_.currentPct * 100.0f);

            logs::info(
                "Recoverable:    {:.2f}%",
                state_.recoverablePct * 100.0f);

            logs::info(
                "Loss:           {:.2f}%",
                state_.lossPct * 100.0f);

            logs::info(
                "================================");

            return true;
        }

        // ========================================
        // Change detection
        // ========================================

        if (!HasChanged(newState)) {
            return false;
        }

        state_ = newState;

        return true;
    }

    const UIState& UIStateManager::GetState() const
    {
        return state_;
    }

    void UIStateManager::Reset()
    {
        state_ = {};
        initialized_ = false;
    }

    bool UIStateManager::HasChanged(
        const UIState& a_newState) const
    {
        const auto changed =
            [](float a_left, float a_right)
            {
                return std::fabs(
                           a_left - a_right) >
                       kEpsilon;
            };

        return
            changed(
                state_.currentHealth,
                a_newState.currentHealth) ||

            changed(
                state_.maxHealth,
                a_newState.maxHealth) ||

            changed(
                state_.recoverableHealth,
                a_newState.recoverableHealth) ||

            changed(
                state_.currentPct,
                a_newState.currentPct) ||

            changed(
                state_.recoverablePct,
                a_newState.recoverablePct) ||

            changed(
                state_.lossPct,
                a_newState.lossPct);
    }
}