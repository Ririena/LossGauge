#include "Gameplay/LossManager.h"

namespace LossGauge
{
    namespace
    {
        constexpr float kHealthEpsilon = 0.05f;
        constexpr float kLossEpsilon = 0.001f;
    }

    LossManager* LossManager::GetSingleton()
    {
        static LossManager instance;
        return std::addressof(instance);
    }

    void LossManager::AddLoss(float a_amount)
    {
        if (a_amount <= 0.0f) {
            return;
        }

        const float maxHealth =
            GetMaxHealth();

        if (maxHealth <= 0.0f) {
            logs::warn(
                "AddLoss: invalid Max Health.");
            return;
        }

        const float lossBefore =
            loss_;

        loss_ += a_amount;

        const float maximumLoss =
            (std::max)(
                0.0f,
                maxHealth - 1.0f);

        loss_ =
            std::clamp(
                loss_,
                0.0f,
                maximumLoss);

        const float actualAdded =
            loss_ - lossBefore;

        // If a recovery cycle already exists,
        // newly gained Loss must also be added
        // to the recovery target.
        
        // This prevents previously accumulated
        // sleep hours from instantly recovering
        // newly received damage.
        if (actualAdded > 0.0f &&
            recoveryBaseLoss_ > 0.0f) {

            recoveryBaseLoss_ +=
                actualAdded;
        }

        logs::info(
            "Loss added: {:.2f} | "
            "Total Loss: {:.2f}",
            actualAdded,
            loss_);
    }

    void LossManager::RecoverLoss(float a_amount)
    {
        if (a_amount <= 0.0f) {
            return;
        }

        loss_ =
            (std::max)(
                0.0f,
                loss_ - a_amount);

        if (loss_ <= kLossEpsilon) {
            loss_ = 0.0f;
            ResetSleepRecoveryProgress();
        }

        logs::info(
            "Loss recovered: {:.2f} | "
            "Remaining Loss: {:.2f}",
            a_amount,
            loss_);
    }

    void LossManager::ResetLoss()
    {
        loss_ = 0.0f;

        ResetSleepRecoveryProgress();

        logs::info(
            "Loss reset.");
    }

    void LossManager::SetLoss(float a_loss)
    {
        if (!std::isfinite(a_loss)) {
            logs::warn(
                "SetLoss: invalid serialized "
                "Loss value.");

            loss_ = 0.0f;

            ResetSleepRecoveryProgress();
            return;
        }

        const float maxHealth =
            GetMaxHealth();

        if (maxHealth > 0.0f) {
            const float maximumLoss =
                (std::max)(
                    0.0f,
                    maxHealth - 1.0f);

            loss_ =
                std::clamp(
                    a_loss,
                    0.0f,
                    maximumLoss);
        }
        else {
            loss_ =
                (std::max)(
                    0.0f,
                    a_loss);
        }

        // Loading old save / restoring Loss:
        // begin a fresh recovery cycle from
        // the restored Loss amount.
        recoveryBaseLoss_ =
            loss_;

        recoveryHours_ =
            0.0f;

        logs::info(
            "Loss restored: {:.2f}",
            loss_);
    }

    void LossManager::RecoverFromSleep(
        float a_sleepHours,
        float a_fullRecoveryHours)
    {
        if (a_sleepHours <= 0.0f) {
            return;
        }

        if (a_fullRecoveryHours <= 0.0f) {
            logs::warn(
                "RecoverFromSleep: invalid "
                "FullRecoveryHours.");
            return;
        }

        if (loss_ <= kLossEpsilon) {
            loss_ = 0.0f;

            ResetSleepRecoveryProgress();
            return;
        }

        // Start recovery cycle

        if (recoveryBaseLoss_ <= kLossEpsilon) {
            recoveryBaseLoss_ =
                loss_;

            recoveryHours_ =
                0.0f;

            logs::info(
                "Sleep recovery cycle started. "
                "Base Loss: {:.2f}",
                recoveryBaseLoss_);
        }

        const float previousHours =
            recoveryHours_;

        const float previousRatio =
            std::clamp(
                previousHours /
                    a_fullRecoveryHours,
                0.0f,
                1.0f);

        recoveryHours_ +=
            a_sleepHours;

        recoveryHours_ =
            std::clamp(
                recoveryHours_,
                0.0f,
                a_fullRecoveryHours);

        const float newRatio =
            std::clamp(
                recoveryHours_ /
                    a_fullRecoveryHours,
                0.0f,
                1.0f);

        // Only recover the NEW percentage
        // reached by this sleep session.

        // Example:
        // 4h / 8h:
        // previous = 0%
        // new      = 50%
        // recover  = 50% of base

        // another 4h:
        // previous = 50%
        // new      = 100%
        // recover  = another 50% of base
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

        const float lossBefore =
            loss_;

        loss_ -=
            actualRecovery;

        loss_ =
            (std::max)(
                0.0f,
                loss_);

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
            previousRatio * 100.0f);

        logs::info(
            "Current Progress:   {:.1f}%",
            newRatio * 100.0f);

        logs::info(
            "Progress Added:     {:.1f}%",
            ratioDelta * 100.0f);

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

        logs::info(
            "================================");

        // Full recovery cycle reached.
        if (newRatio >= 1.0f ||
            loss_ <= kLossEpsilon) {

            loss_ = 0.0f;

            logs::info(
                "Sleep recovery cycle complete.");

            ResetSleepRecoveryProgress();
        }
    }

    void LossManager::ResetSleepRecoveryProgress()
    {
        recoveryBaseLoss_ =
            0.0f;

        recoveryHours_ =
            0.0f;
    }

    float LossManager::GetLoss() const
    {
        return loss_;
    }

    float LossManager::GetRecoveryBaseLoss() const
    {
        return recoveryBaseLoss_;
    }

    float LossManager::GetRecoveryHours() const
    {
        return recoveryHours_;
    }

    float LossManager::GetCurrentHealth() const
    {
        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            logs::warn(
                "GetCurrentHealth: "
                "PlayerCharacter is null.");

            return 0.0f;
        }

        auto* actorValueOwner =
            player->AsActorValueOwner();

        if (!actorValueOwner) {
            logs::warn(
                "GetCurrentHealth: "
                "ActorValueOwner is null.");

            return 0.0f;
        }

        return actorValueOwner->
            GetActorValue(
                RE::ActorValue::kHealth);
    }

    float LossManager::GetPermanentHealth() const
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

    float LossManager::GetMaxHealth() const
    {
        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            logs::warn(
                "GetMaxHealth: "
                "PlayerCharacter is null.");

            return 0.0f;
        }

        return player->
            GetActorValueMax(
                RE::ActorValue::kHealth);
    }

    float LossManager::GetRecoverableHealth() const
    {
        const float maxHealth =
            GetMaxHealth();

        if (maxHealth <= 0.0f) {
            return 0.0f;
        }

        return (std::max)(
            1.0f,
            maxHealth - loss_);
    }

    bool LossManager::ClampCurrentHealth()
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

        if (recoverableHealth <= 0.0f) {
            return false;
        }

        if (currentHealth <=
            recoverableHealth +
                kHealthEpsilon) {

            return false;
        }

        const float excessHealth =
            currentHealth -
            recoverableHealth;

        logs::info(
            "================================");

        logs::info(
            "Healing Cap");

        logs::info(
            "--------------------------------");

        logs::info(
            "Current HP:     {:.2f}",
            currentHealth);

        logs::info(
            "Recoverable HP: {:.2f}",
            recoverableHealth);

        logs::info(
            "Excess HP:      {:.2f}",
            excessHealth);

        actorValueOwner->
            DamageActorValue(
                RE::ActorValue::kHealth,
                excessHealth);

        const float healthAfterClamp =
            actorValueOwner->
                GetActorValue(
                    RE::ActorValue::kHealth);

        logs::info(
            "Health after clamp: {:.2f}",
            healthAfterClamp);

        logs::info(
            "================================");

        return true;
    }
}