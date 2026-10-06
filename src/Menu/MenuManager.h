#pragma once

namespace LossGauge
{
    class MenuManager
    {
    public:
        static void Register();

    private:
        static void __stdcall RenderSettings();
        static void __stdcall RenderDebug();

        static void InitializePendingSettings();
        static void ResetPendingSettings();

        static bool registered_;

        static bool pendingInitialized_;

        static float pendingLossRatio_;

        static bool pendingSleepRecovery_;
        static float pendingFullRecoveryHours_;

        static bool pendingNaturalRegen_;
    };
}