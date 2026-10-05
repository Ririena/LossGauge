#include "Hooks/PlayerUpdateHook.h"

#include "Config/ConfigManager.h"
#include "Gameplay/LossManager.h"

namespace LossGauge
{
    void PlayerUpdateHook::Install()
    {
        REL::Relocation<std::uintptr_t> vtable{
            RE::PlayerCharacter::VTABLE[0]
        };

        originalUpdate_ =
            vtable.write_vfunc(
                0xAD,
                Update);

        logs::info(
            "PlayerUpdateHook installed.");
    }

    void PlayerUpdateHook::ResetHealthSnapshot()
    {
        auto* manager =
            LossManager::GetSingleton();

        const float currentHealth =
            manager->GetCurrentHealth();

        previousHealth_ =
            currentHealth;

        initialized_ =
            currentHealth > 0.0f;

        clampCooldown_ =
            0.0f;

        logs::info(
            "Realtime health snapshot reset: {:.2f}",
            previousHealth_);
    }

    float PlayerUpdateHook::GetLastGameHours()
    {
        return lastGameHours_;
    }

    void PlayerUpdateHook::ResetGameTimeSnapshot()
    {
        auto* calendar =
            RE::Calendar::GetSingleton();

        if (!calendar) {
            logs::warn(
                "ResetGameTimeSnapshot: "
                "Calendar is null.");

            gameTimeInitialized_ =
                false;

            return;
        }

        lastGameHours_ =
            calendar->GetHoursPassed();

        gameTimeInitialized_ =
            true;

        logs::info(
            "Game-time snapshot reset: {:.4f}",
            lastGameHours_);
    }

    void PlayerUpdateHook::Update(
        RE::PlayerCharacter* a_player,
        float a_delta)
    {
        originalUpdate_(
            a_player,
            a_delta);

        if (!a_player) {
            return;
        }

        // Game-time snapshot

        auto* calendar =
            RE::Calendar::GetSingleton();

        if (calendar) {
            const float currentGameHours =
                calendar->GetHoursPassed();

            if (!gameTimeInitialized_) {
                lastGameHours_ =
                    currentGameHours;

                gameTimeInitialized_ =
                    true;
            }
            else {
                lastGameHours_ =
                    currentGameHours;
            }
        }

        // Existing Loss Gauge gameplay

        auto* manager =
            LossManager::GetSingleton();

        auto* config =
            ConfigManager::GetSingleton();

        if (clampCooldown_ > 0.0f) {
            clampCooldown_ -=
                a_delta;

            if (clampCooldown_ < 0.0f) {
                clampCooldown_ =
                    0.0f;
            }
        }

        const float currentHealth =
            manager->GetCurrentHealth();

        if (currentHealth <= 0.0f) {
            previousHealth_ =
                currentHealth;

            initialized_ =
                false;

            clampCooldown_ =
                0.0f;

            return;
        }

        if (!initialized_) {
            previousHealth_ =
                currentHealth;

            initialized_ =
                true;

            clampCooldown_ =
                0.0f;

            manager->
                ClampCurrentHealth();

            previousHealth_ =
                manager->
                    GetCurrentHealth();

            return;
        }

        const float healthDelta =
            currentHealth -
            previousHealth_;

        // Damage

        if (healthDelta < -kDeltaEpsilon) {
            const float damage =
                -healthDelta;

            const float lossRatio =
                config->
                    GetLossRatio();

            const float lossGained =
                damage *
                lossRatio;

            manager->
                AddLoss(
                    lossGained);

            logs::info(
                "================================");

            logs::info(
                "Realtime Player Damage");

            logs::info(
                "--------------------------------");

            logs::info(
                "Previous HP:    {:.2f}",
                previousHealth_);

            logs::info(
                "Current HP:     {:.2f}",
                currentHealth);

            logs::info(
                "Damage:         {:.2f}",
                damage);

            logs::info(
                "Loss Ratio:     {:.0f}%",
                lossRatio * 100.0f);

            logs::info(
                "Loss Gained:    {:.2f}",
                lossGained);

            logs::info(
                "Total Loss:     {:.2f}",
                manager->
                    GetLoss());

            logs::info(
                "Max HP:         {:.2f}",
                manager->
                    GetMaxHealth());

            logs::info(
                "Recoverable HP: {:.2f}",
                manager->
                    GetRecoverableHealth());

            logs::info(
                "================================");

            clampCooldown_ =
                0.0f;
        }

        // Healing clamp

        const float recoverableHealth =
            manager->
                GetRecoverableHealth();

        const float excessHealth =
            currentHealth -
            recoverableHealth;

        const bool exceedsCap =
            excessHealth >
            kDeltaEpsilon;

        const bool healthIncreased =
            healthDelta >
            kDeltaEpsilon;

        const bool largeHeal =
            healthIncreased &&
            healthDelta >=
                kLargeHealThreshold;

        bool healthWasClamped =
            false;

        if (exceedsCap) {
            if (largeHeal) {
                healthWasClamped =
                    manager->
                        ClampCurrentHealth();

                clampCooldown_ =
                    kClampInterval;

                if (healthWasClamped) {
                    logs::info(
                        "Large healing "
                        "clamped immediately.");
                }
            }
            else if (
                clampCooldown_ <= 0.0f) {

                healthWasClamped =
                    manager->
                        ClampCurrentHealth();

                if (healthWasClamped) {
                    clampCooldown_ =
                        kClampInterval;

                    logs::info(
                        "Continuous healing "
                        "clamped. "
                        "Next correction in "
                        "{:.2f}s.",
                        kClampInterval);
                }
            }
        }

        const float finalHealth =
            manager->
                GetCurrentHealth();

        if (healthWasClamped) {
            logs::info(
                "Healing clamp baseline "
                "synchronized: {:.2f}",
                finalHealth);
        }

        previousHealth_ =
            finalHealth;
    }
}