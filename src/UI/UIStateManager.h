#pragma once

namespace LossGauge
{
    struct UIState
    {
        float currentHealth{ 0.0f };
        float maxHealth{ 0.0f };
        float recoverableHealth{ 0.0f };

        float currentPct{ 0.0f };
        float recoverablePct{ 0.0f };
        float lossPct{ 0.0f };
    };

    class UIStateManager
    {
    public:
        static UIStateManager* GetSingleton();

        [[nodiscard]] bool Update();

        [[nodiscard]] const UIState& GetState() const;

        void Reset();

    private:
        UIStateManager() = default;

        UIStateManager(const UIStateManager&) = delete;
        UIStateManager(UIStateManager&&) = delete;

        UIStateManager& operator=(const UIStateManager&) = delete;
        UIStateManager& operator=(UIStateManager&&) = delete;

        [[nodiscard]] bool HasChanged(const UIState& a_newState) const;

        static constexpr float kEpsilon = 0.0001f;

        UIState state_{};
        bool initialized_{ false };
    };
}