#pragma once

#include "UI/UIStateManager.h"

namespace LossGauge
{
    class ScaleformBridge
    {
    public:
        static ScaleformBridge* GetSingleton();

        [[nodiscard]] bool SendState(
            const UIState& a_state);

        void UpdateLifecycle();

        void Reset();

    private:
        ScaleformBridge() = default;

        ScaleformBridge(
            const ScaleformBridge&) = delete;

        ScaleformBridge(
            ScaleformBridge&&) = delete;

        ScaleformBridge& operator=(
            const ScaleformBridge&) = delete;

        ScaleformBridge& operator=(
            ScaleformBridge&&) = delete;

        [[nodiscard]] bool ShouldSendState(
            const UIState& a_state) const;

        static constexpr float
            kCurrentPctThreshold = 0.001f;

        static constexpr float
            kImportantPctEpsilon = 0.0001f;

        bool hudAvailable_{ false };

        bool stateSent_{ false };

        UIState lastSentState_{};
    };
}