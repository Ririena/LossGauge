#include "UI/UIStateManager.h"

#include "Gameplay/LossManager.h"

namespace LossGauge
{
    UIStateManager*
    UIStateManager::GetSingleton()
    {
        static UIStateManager singleton;
        return &singleton;
    }

    bool UIStateManager::Update()
    {
        auto* manager =
            LossManager::
                GetSingleton();

        if (!manager) {
            return false;
        }

        UIState newState{};

        newState.currentHealth =
            manager->
                GetCurrentHealth();

        newState.maxHealth =
            manager->
                GetMaxHealth();

        newState.recoverableHealth =
            manager->
                GetRecoverableHealth();

        // Protect the UI from invalid values.
        if (!std::isfinite(
                newState.currentHealth) ||
            !std::isfinite(
                newState.maxHealth) ||
            !std::isfinite(
                newState.recoverableHealth)) {

            return false;
        }

        // A percentage cannot be calculated
        // safely without a positive Max HP.
        if (newState.maxHealth <= kEpsilon) {
            return false;
        }

        // ========================================
        // Normalize
        // ========================================

        newState.currentPct =
            newState.currentHealth /
            newState.maxHealth;

        newState.recoverablePct =
            newState.recoverableHealth /
            newState.maxHealth;

        // Loss is the portion of Max HP that
        // can no longer currently be recovered.
        newState.lossPct =
            1.0f -
            newState.recoverablePct;

        // ========================================
        // Safety Clamp
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
        // beyond the recoverable portion.
        newState.currentPct =
            (std::min)(
                newState.currentPct,
                newState.recoverablePct);

        // ========================================
        // Change Detection
        // ========================================

        if (!initialized_) {
            state_ = newState;
            initialized_ = true;

            logs::info(
                "UI state initialized.");

            logs::info(
                "Current: {:.2f} / {:.2f} ({:.1f}%)",
                state_.currentHealth,
                state_.maxHealth,
                state_.currentPct * 100.0f);

            logs::info(
                "Recoverable: {:.2f} ({:.1f}%)",
                state_.recoverableHealth,
                state_.recoverablePct * 100.0f);

            logs::info(
                "Loss: {:.1f}%",
                state_.lossPct * 100.0f);

            return true;
        }

        if (!HasChanged(newState)) {
            return false;
        }

        state_ = newState;

        return true;
    }

    const UIState&
    UIStateManager::GetState() const
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
            [](float a_left,
               float a_right)
            {
                return std::fabs(
                           a_left -
                           a_right) >
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