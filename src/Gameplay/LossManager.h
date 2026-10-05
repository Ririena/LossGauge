#pragma once

namespace LossGauge
{
    class LossManager
    {
    public:
        static LossManager* GetSingleton();

        // ========================================
        // Loss
        // ========================================

        [[nodiscard]]
        float GetLoss() const;

        void SetLoss(float a_loss);

        void AddLoss(float a_amount);

        void ResetLoss();

        // ========================================
        // Health
        // ========================================

        [[nodiscard]]
        float GetCurrentHealth() const;

        [[nodiscard]]
        float GetPermanentHealth() const;

        [[nodiscard]]
        float GetMaxHealth() const;

        [[nodiscard]]
        float GetRecoverableHealth() const;

        // Returns true only when current Health
        // was actually reduced to the recoverable
        // Health ceiling.
        [[nodiscard]]
        bool ClampCurrentHealth();

        // ========================================
        // Sleep Recovery
        // ========================================

        void RecoverFromSleep(
            float a_sleepHours,
            float a_fullRecoveryHours);

        void ResetSleepRecoveryProgress();

        [[nodiscard]]
        float GetRecoveryBaseLoss() const;

        [[nodiscard]]
        float GetRecoveryHours() const;

        // ========================================
        // Serialization
        // ========================================

        // Restore the complete runtime state
        // without resetting cumulative sleep
        // recovery progress.
        void RestoreState(
            float a_loss,
            float a_recoveryBaseLoss,
            float a_recoveryHours);

    private:
        LossManager() = default;

        LossManager(
            const LossManager&) = delete;

        LossManager(
            LossManager&&) = delete;

        LossManager& operator=(
            const LossManager&) = delete;

        LossManager& operator=(
            LossManager&&) = delete;

        // Current Loss Gauge amount.
        float loss_{ 0.0f };

        // Original Loss amount used as the basis
        // of the current cumulative sleep cycle.
        float recoveryBaseLoss_{ 0.0f };

        // Total sleep hours accumulated during
        // the current recovery cycle.
        float recoveryHours_{ 0.0f };
    };
}