#pragma once

namespace LossGauge
{
    class LossManager
    {
    public:
        static LossManager* GetSingleton();

        void AddLoss(float a_amount);
        void RecoverLoss(float a_amount);
        void ResetLoss();

        void SetLoss(float a_loss);

        // Sleep recovery
        void RecoverFromSleep(
            float a_sleepHours,
            float a_fullRecoveryHours);

        void ResetSleepRecoveryProgress();

        [[nodiscard]] float GetLoss() const;
        [[nodiscard]] float GetCurrentHealth() const;
        [[nodiscard]] float GetPermanentHealth() const;
        [[nodiscard]] float GetMaxHealth() const;
        [[nodiscard]] float GetRecoverableHealth() const;

        [[nodiscard]] float GetRecoveryBaseLoss() const;
        [[nodiscard]] float GetRecoveryHours() const;

        bool ClampCurrentHealth();

    private:
        LossManager() = default;
        LossManager(const LossManager&) = delete;
        LossManager(LossManager&&) = delete;

        LossManager& operator=(
            const LossManager&) = delete;

        LossManager& operator=(
            LossManager&&) = delete;

        float loss_{ 0.0f };

        // Sleep recovery cycle.
        float recoveryBaseLoss_{ 0.0f };
        float recoveryHours_{ 0.0f };
    };
}