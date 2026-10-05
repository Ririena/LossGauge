#include "Gameplay/LossManager.h"

namespace LossGauge
{
    namespace
    {
        constexpr float kEpsilon =
            0.001f;
    }

    LossManager*
    LossManager::GetSingleton()
    {
        static LossManager instance;

        return std::addressof(instance);
    }

    // ============================================
    // Loss
    // ============================================

    float LossManager::GetLoss() const
    {
        return loss_;
    }

    void LossManager::SetLoss(
        float a_loss)
    {
        if (!std::isfinite(a_loss)) {
            a_loss =
                0.0f;
        }

        const float maxHealth =
            GetMaxHealth();

        loss_ =
            std::clamp(
                a_loss,
                0.0f,
                (std::max)(
                    0.0f,
                    maxHealth));

        // SetLoss() represents a fresh Loss state.
        //
        // It intentionally resets cumulative
        // recovery progress.
        //
        // Serialization v2 MUST use RestoreState()
        // instead.
        recoveryBaseLoss_ =
            loss_;

        recoveryHours_ =
            0.0f;

        logs::info(
            "Loss restored: {:.2f}",
            loss_);
    }

    void LossManager::AddLoss(
        float a_amount)
    {
        if (!std::isfinite(a_amount)) {
            return;
        }

        if (a_amount <= 0.0f) {
            return;
        }

        const float maxHealth =
            GetMaxHealth();

        if (maxHealth <= 0.0f) {
            return;
        }

        const float previousLoss =
            loss_;

        loss_ =
            std::clamp(
                loss_ + a_amount,
                0.0f,
                maxHealth);

        const float actualAdded =
            loss_ -
            previousLoss;

        if (actualAdded <= kEpsilon) {
            return;
        }

        // If a cumulative sleep recovery cycle is
        // already active, new damage must not gain
        // free recovery from hours slept before
        // that damage occurred.
        //
        // Example:
        //
        // Loss 20
        // Sleep 4 / 8h
        // Loss becomes 10
        //
        // Then gain 5 new Loss.
        //
        // Recovery base becomes:
        //
        // 20 + 5 = 25
        //
        // while recoveryHours remains 4.
        if (recoveryHours_ > kEpsilon) {
            recoveryBaseLoss_ +=
                actualAdded;
        }
        else {
            recoveryBaseLoss_ =
                loss_;
        }

        logs::info(
            "Loss added: {:.2f} | "
            "Total Loss: {:.2f}",
            actualAdded,
            loss_);
    }

    void LossManager::ResetLoss()
    {
        loss_ =
            0.0f;

        ResetSleepRecoveryProgress();

        logs::info(
            "Loss reset.");
    }

    // ============================================
    // Health
    // ============================================

    float LossManager::
    GetCurrentHealth() const
    {
        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            return 0.0f;
        }

        auto* actorValueOwner =
            player->AsActorValueOwner();

        if (!actorValueOwner) {
            return 0.0f;
        }

        return actorValueOwner->
            GetActorValue(
                RE::ActorValue::kHealth);
    }

    float LossManager::
    GetPermanentHealth() const
    {
        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            return 0.0f;
        }

        auto* actorValueOwner =
            player->AsActorValueOwner();

        if (!actorValueOwner) {
            return 0.0f;
        }

        return actorValueOwner->
            GetPermanentActorValue(
                RE::ActorValue::kHealth);
    }

    float LossManager::
    GetMaxHealth() const
    {
        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            return 0.0f;
        }

        return player->
            GetActorValueMax(
                RE::ActorValue::kHealth);
    }

    float LossManager::
    GetRecoverableHealth() const
    {
        const float maxHealth =
            GetMaxHealth();

        return (std::max)(
            0.0f,
            maxHealth - loss_);
    }

    bool LossManager::
    ClampCurrentHealth()
    {
        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            return false;
        }

        auto* actorValueOwner =
            player->AsActorValueOwner();

        if (!actorValueOwner) {
            return false;
        }

        const float currentHealth =
            actorValueOwner->
                GetActorValue(
                    RE::ActorValue::kHealth);

        const float recoverableHealth =
            GetRecoverableHealth();

        const float excessHealth =
            currentHealth -
            recoverableHealth;

        // Current Health is already inside
        // the allowed ceiling.
        if (excessHealth <= kEpsilon) {
            return false;
        }

        // Reduce only the amount above the
        // Recoverable HP ceiling.
        actorValueOwner->
            DamageActorValue(
                RE::ActorValue::kHealth,
                excessHealth);

        return true;
    }

    // ============================================
    // Sleep Recovery
    // ============================================

    void LossManager::
    RecoverFromSleep(
        float a_sleepHours,
        float a_fullRecoveryHours)
    {
        if (!std::isfinite(
                a_sleepHours) ||
            !std::isfinite(
                a_fullRecoveryHours)) {

            return;
        }

        if (a_sleepHours <= 0.0f ||
            a_fullRecoveryHours <= 0.0f) {

            return;
        }

        // Nothing to recover.
        if (loss_ <= kEpsilon) {
            loss_ =
                0.0f;

            ResetSleepRecoveryProgress();

            return;
        }

        // Start a new recovery cycle when no
        // valid cycle currently exists.
        if (recoveryBaseLoss_ <=
            kEpsilon) {

            recoveryBaseLoss_ =
                loss_;

            recoveryHours_ =
                0.0f;
        }

        const float lossBefore =
            loss_;

        const float previousHours =
            recoveryHours_;

        const float previousRatio =
            std::clamp(
                previousHours /
                    a_fullRecoveryHours,
                0.0f,
                1.0f);

        const float newHours =
            (std::min)(
                previousHours +
                    a_sleepHours,
                a_fullRecoveryHours);

        const float newRatio =
            std::clamp(
                newHours /
                    a_fullRecoveryHours,
                0.0f,
                1.0f);

        const float ratioDelta =
            (std::max)(
                0.0f,
                newRatio -
                    previousRatio);

        const float requestedRecovery =
            recoveryBaseLoss_ *
            ratioDelta;

        const float actualRecovery =
            (std::min)(
                loss_,
                requestedRecovery);

        loss_ -=
            actualRecovery;

        if (loss_ <= kEpsilon) {
            loss_ =
                0.0f;
        }

        recoveryHours_ =
            newHours;

        logs::info(
            "================================");

        logs::info(
            "Cumulative Sleep Recovery");

        logs::info(
            "--------------------------------");

        logs::info(
            "Sleep This Session: {:.2f}h",
            a_sleepHours);

        logs::info(
            "Previous Hours:     {:.2f}h",
            previousHours);

        logs::info(
            "Total Hours:        {:.2f}h",
            recoveryHours_);

        logs::info(
            "Full Recovery:      {:.2f}h",
            a_fullRecoveryHours);

        logs::info(
            "Previous Progress:  {:.1f}%",
            previousRatio *
                100.0f);

        logs::info(
            "Current Progress:   {:.1f}%",
            newRatio *
                100.0f);

        logs::info(
            "Progress Added:     {:.1f}%",
            ratioDelta *
                100.0f);

        logs::info(
            "Recovery Base Loss: {:.2f}",
            recoveryBaseLoss_);

        logs::info(
            "Loss Before:        {:.2f}",
            lossBefore);

        logs::info(
            "Loss Recovered:     {:.2f}",
            actualRecovery);

        logs::info(
            "Loss After:         {:.2f}",
            loss_);

        // Once recovery reaches 100%, the cycle
        // is finished.
        if (loss_ <= kEpsilon ||
            recoveryHours_ >=
                a_fullRecoveryHours -
                    kEpsilon) {

            logs::info(
                "Sleep recovery cycle complete.");

            ResetSleepRecoveryProgress();
        }

        logs::info(
            "================================");
    }

    void LossManager::
    ResetSleepRecoveryProgress()
    {
        recoveryBaseLoss_ =
            0.0f;

        recoveryHours_ =
            0.0f;
    }

    float LossManager::
    GetRecoveryBaseLoss() const
    {
        return recoveryBaseLoss_;
    }

    float LossManager::
    GetRecoveryHours() const
    {
        return recoveryHours_;
    }

    // ============================================
    // Serialization
    // ============================================

    void LossManager::
    RestoreState(
        float a_loss,
        float a_recoveryBaseLoss,
        float a_recoveryHours)
    {
        // ----------------------------------------
        // Loss validation
        // ----------------------------------------

        if (!std::isfinite(a_loss)) {
            a_loss =
                0.0f;
        }

        const float maxHealth =
            GetMaxHealth();

        loss_ =
            std::clamp(
                a_loss,
                0.0f,
                (std::max)(
                    0.0f,
                    maxHealth));

        // ----------------------------------------
        // Recovery base validation
        // ----------------------------------------

        if (!std::isfinite(
                a_recoveryBaseLoss)) {

            a_recoveryBaseLoss =
                0.0f;
        }

        recoveryBaseLoss_ =
            (std::max)(
                0.0f,
                a_recoveryBaseLoss);

        // Recovery base cannot meaningfully exceed
        // the player's current maximum Health.
        recoveryBaseLoss_ =
            (std::min)(
                recoveryBaseLoss_,
                (std::max)(
                    0.0f,
                    maxHealth));

        // ----------------------------------------
        // Recovery hours validation
        // ----------------------------------------

        if (!std::isfinite(
                a_recoveryHours)) {

            a_recoveryHours =
                0.0f;
        }

        recoveryHours_ =
            (std::max)(
                0.0f,
                a_recoveryHours);

        // ----------------------------------------
        // Defensive state correction
        // ----------------------------------------

        // No Loss means no active recovery cycle.
        if (loss_ <= kEpsilon) {
            loss_ =
                0.0f;

            ResetSleepRecoveryProgress();
        }
        else if (
            recoveryHours_ > kEpsilon &&
            recoveryBaseLoss_ <= kEpsilon) {

            // Corrupt/incomplete recovery state.
            // Restart recovery from current Loss.
            logs::warn(
                "Invalid recovery state detected. "
                "Restarting sleep recovery cycle.");

            recoveryBaseLoss_ =
                loss_;

            recoveryHours_ =
                0.0f;
        }
        else if (
            recoveryHours_ <= kEpsilon &&
            recoveryBaseLoss_ <= kEpsilon) {

            // Valid fresh state with Loss but no
            // recovery session started yet.
            recoveryBaseLoss_ =
                loss_;

            recoveryHours_ =
                0.0f;
        }

        logs::info(
            "Loss Gauge state restored:");

        logs::info(
            "  Loss:              {:.2f}",
            loss_);

        logs::info(
            "  Recovery Base:     {:.2f}",
            recoveryBaseLoss_);

        logs::info(
            "  Recovery Hours:    {:.2f}",
            recoveryHours_);
    }
}