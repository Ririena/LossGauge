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

        bool hudAvailable_{ false };
    };
}