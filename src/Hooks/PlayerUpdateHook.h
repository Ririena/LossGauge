#pragma once

namespace LossGauge
{
    class PlayerUpdateHook
    {
    public:
        static void Install();
        static void ResetHealthSnapshot();

        static float GetLastGameHours();
        static void ResetGameTimeSnapshot();

    private:
        static void Update(
            RE::PlayerCharacter* a_player,
            float a_delta);

        static inline REL::Relocation<
            decltype(Update)> originalUpdate_;

        static inline float previousHealth_{ 0.0f };
        static inline bool initialized_{ false };
        static inline float clampCooldown_{ 0.0f };

        static inline float lastGameHours_{ 0.0f };
        static inline bool gameTimeInitialized_{ false };

        static constexpr float kDeltaEpsilon = 0.001f;
        static constexpr float kLargeHealThreshold = 1.0f;
        static constexpr float kClampInterval = 0.10f;
    };
}