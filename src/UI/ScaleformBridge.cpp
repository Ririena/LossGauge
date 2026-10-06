#include "UI/ScaleformBridge.h"

#include "Config/ConfigManager.h"

namespace LossGauge
{
    namespace
    {
        bool IsDebugLoggingEnabled()
        {
            const auto* config =
                ConfigManager::
                    GetSingleton();

            return
                config &&
                config->
                    IsDebugLoggingEnabled();
        }
    }

    ScaleformBridge*
        ScaleformBridge::GetSingleton()
    {
        static ScaleformBridge singleton;
        return &singleton;
    }

    bool ScaleformBridge::SendState(
        const UIState& a_state)
    {
        // ========================================
        // Validate State
        // ========================================

        if (!std::isfinite(
                a_state.currentPct) ||
            !std::isfinite(
                a_state.recoverablePct) ||
            !std::isfinite(
                a_state.lossPct)) {

            return false;
        }

        // ========================================
        // UI
        // ========================================

        auto* ui =
            RE::UI::GetSingleton();

        if (!ui) {
            hudAvailable_ = false;
            stateSent_ = false;

            return false;
        }

        auto movie =
            ui->GetMovieView(
                RE::HUDMenu::MENU_NAME);

        if (!movie) {
            hudAvailable_ = false;
            stateSent_ = false;

            return false;
        }

        // ========================================
        // HUD Detection
        // ========================================

        if (!hudAvailable_) {
            hudAvailable_ = true;

            // This may be a newly created HUD.
            // Force the first state through.
            stateSent_ = false;

            if (IsDebugLoggingEnabled()) {
                logs::info(
                    "Loss Gauge Scaleform: "
                    "HUD movie detected.");
            }
        }

        // ========================================
        // Transmission Filter
        // ========================================

        if (!ShouldSendState(
                a_state)) {

            return false;
        }

        // ========================================
        // GFx Arguments
        // ========================================

        RE::GFxValue args[2];

        args[0].SetNumber(
            static_cast<double>(
                a_state.currentPct));

        args[1].SetNumber(
            static_cast<double>(
                a_state.recoverablePct));

        // ========================================
        // ActionScript Call
        // ========================================

        movie->InvokeNoReturn(
            "_lossGauge.setState",
            args,
            2);

        // ========================================
        // Transmission Cache
        // ========================================

        lastSentState_ =
            a_state;

        stateSent_ =
            true;

        // ========================================
        // Debug
        // ========================================

        if (IsDebugLoggingEnabled()) {
            logs::info(
                "Loss Gauge Scaleform state sent: "
                "current={:.2f}% "
                "recoverable={:.2f}% "
                "loss={:.2f}%",
                a_state.currentPct *
                    100.0f,
                a_state.recoverablePct *
                    100.0f,
                a_state.lossPct *
                    100.0f);
        }

        return true;
    }

    void ScaleformBridge::UpdateLifecycle()
    {
        // ========================================
        // Current HUD State
        // ========================================

        auto* ui =
            RE::UI::GetSingleton();

        bool hudExists =
            false;

        if (ui) {
            auto movie =
                ui->GetMovieView(
                    RE::HUDMenu::MENU_NAME);

            hudExists =
                static_cast<bool>(
                    movie);
        }

        // ========================================
        // HUD Disappeared
        // ========================================

        if (!hudExists) {
            if (hudAvailable_) {
                hudAvailable_ =
                    false;

                // Do not consider the previous
                // movie's transmission valid for
                // the next HUD instance.
                stateSent_ =
                    false;

                if (IsDebugLoggingEnabled()) {
                    logs::info(
                        "Loss Gauge Scaleform: "
                        "HUD movie unavailable.");
                }
            }

            return;
        }

        // ========================================
        // HUD Already Available
        // ========================================

        if (hudAvailable_) {
            return;
        }

        // ========================================
        // HUD Appeared / Reappeared
        // ========================================

        hudAvailable_ =
            true;

        // New HUD instance must receive state
        // regardless of transmission threshold.
        stateSent_ =
            false;

        if (IsDebugLoggingEnabled()) {
            logs::info(
                "Loss Gauge Scaleform: "
                "HUD movie detected.");
        }

        // ========================================
        // Force Current State
        // ========================================

        auto* uiStateManager =
            UIStateManager::
                GetSingleton();

        if (!uiStateManager) {
            return;
        }

        // Update() may return false because the
        // gameplay values have not changed.
        //
        // That is fine. We need the manager's
        // current cached state, not necessarily
        // a newly changed state.
        (void)uiStateManager->
            Update();

        const auto& state =
            uiStateManager->
                GetState();

        (void)SendState(
            state);
    }

    bool ScaleformBridge::ShouldSendState(
        const UIState& a_state) const
    {
        // First state for a HUD instance must
        // always be transmitted.
        if (!stateSent_) {
            return true;
        }

        const float currentDelta =
            std::fabs(
                a_state.currentPct -
                lastSentState_.currentPct);

        const float recoverableDelta =
            std::fabs(
                a_state.recoverablePct -
                lastSentState_.
                    recoverablePct);

        const float lossDelta =
            std::fabs(
                a_state.lossPct -
                lastSentState_.lossPct);

        // ========================================
        // Recoverable / Loss
        // ========================================

        if (recoverableDelta >
            kImportantPctEpsilon) {

            return true;
        }

        if (lossDelta >
            kImportantPctEpsilon) {

            return true;
        }

        // ========================================
        // Current HP
        // ========================================

        if (currentDelta >=
            kCurrentPctThreshold) {

            return true;
        }

        return false;
    }

    void ScaleformBridge::Reset()
    {
        hudAvailable_ =
            false;

        stateSent_ =
            false;

        lastSentState_ =
            {};
    }
}