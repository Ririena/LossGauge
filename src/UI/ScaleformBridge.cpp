#include "UI/ScaleformBridge.h"

#include "Config/ConfigManager.h"

namespace LossGauge
{
    ScaleformBridge*
        ScaleformBridge::GetSingleton()
    {
        static ScaleformBridge singleton;
        return &singleton;
    }

    bool ScaleformBridge::SendState(
        const UIState& a_state)
    {
        auto* ui =
            RE::UI::GetSingleton();

        if (!ui) {
            hudAvailable_ = false;
            return false;
        }

        auto movie =
            ui->GetMovieView(
                RE::HUDMenu::MENU_NAME);

        if (!movie) {
            hudAvailable_ = false;
            return false;
        }

        if (!hudAvailable_) {
            hudAvailable_ = true;

            if (ConfigManager::
                    GetSingleton()->
                    IsDebugLoggingEnabled()) {
                logs::info(
                    "Loss Gauge Scaleform: "
                    "HUD movie detected.");
            }
        }

        RE::GFxValue args[2];

        args[0].SetNumber(
            static_cast<double>(
                a_state.currentPct));

        args[1].SetNumber(
            static_cast<double>(
                a_state.recoverablePct));

        movie->InvokeNoReturn(
            "_lossGauge.setState",
            args,
            2);

        if (ConfigManager::
                GetSingleton()->
                IsDebugLoggingEnabled()) {
            logs::info(
                "Loss Gauge Scaleform state sent: "
                "current={:.2f}% "
                "recoverable={:.2f}% "
                "loss={:.2f}%",
                a_state.currentPct * 100.0f,
                a_state.recoverablePct * 100.0f,
                a_state.lossPct * 100.0f);
        }

        return true;
    }

    void ScaleformBridge::Reset()
    {
        hudAvailable_ = false;
    }
}