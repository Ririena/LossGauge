#include "Hooks/PlayerUpdateHook.h"

#include "Config/ConfigManager.h"
#include "Gameplay/LossManager.h"

namespace LossGauge
{
    namespace
    {
        constexpr float kDeltaEpsilon = 0.001f;
        constexpr float kLargeHealThreshold = 1.0f;
        constexpr float kClampInterval = 0.10f;

        bool IsDebugLoggingEnabled()
        {
            const auto* config = ConfigManager::GetSingleton();
            return config && config->IsDebugLoggingEnabled();
        }
    }


    void PlayerUpdateHook::Install()
    {
        REL::Relocation<std::uintptr_t> vtable{
            RE::PlayerCharacter::VTABLE[0]
        };

        originalUpdate_ = vtable.write_vfunc(0xAD, Update);

        logs::info("PlayerUpdateHook installed.");
    }


    void PlayerUpdateHook::Update(
        RE::PlayerCharacter* a_player,
        float a_delta)
    {
        // Always run Skyrim's original PlayerCharacter::Update first.
        originalUpdate_(a_player, a_delta);

        if (!a_player) {
            return;
        }

        auto* manager = LossManager::GetSingleton();
        auto* config = ConfigManager::GetSingleton();

        if (!manager || !config) {
            return;
        }

        auto* actorValueOwner = a_player->AsActorValueOwner();

        if (!actorValueOwner) {
            return;
        }


        // =========================================================
        // GAME-TIME SNAPSHOT
        //
        // Used by SleepEventHandler to determine how many in-game
        // hours actually passed while the player was sleeping.
        // =========================================================

        if (auto* calendar = RE::Calendar::GetSingleton()) {
            const float currentGameHours =
                calendar->GetHoursPassed();

            if (std::isfinite(currentGameHours)) {
                lastGameHours_ = currentGameHours;
                gameTimeInitialized_ = true;
            }
        }


        // =========================================================
        // CURRENT HEALTH
        // =========================================================

        const float currentHealth =
            actorValueOwner->GetActorValue(
                RE::ActorValue::kHealth);

        if (!std::isfinite(currentHealth)) {
            return;
        }


        // =========================================================
        // FIRST-FRAME INITIALIZATION
        // =========================================================

        if (!initialized_) {
            previousHealth_ = currentHealth;
            initialized_ = true;
            clampCooldown_ = 0.0f;

            if (IsDebugLoggingEnabled()) {
                logs::info(
                    "PlayerUpdateHook health baseline initialized: {:.2f}",
                    previousHealth_);
            }

            return;
        }


        // =========================================================
        // CLAMP COOLDOWN
        // =========================================================

        if (std::isfinite(a_delta) && a_delta > 0.0f) {
            clampCooldown_ =
                (std::max)(0.0f, clampCooldown_ - a_delta);
        }


        // =========================================================
        // HEALTH DELTA
        // =========================================================

        const float healthDelta =
            currentHealth - previousHealth_;


        // =========================================================
        // DAMAGE DETECTION
        //
        // Negative health delta means Skyrim reduced player HP.
        //
        // Loss added:
        //
        //     Damage * LossRatio
        // =========================================================

        if (healthDelta < -kDeltaEpsilon) {
            const float damage =
                -healthDelta;

            const float lossRatio =
                config->GetLossRatio();

            const float lossAdded =
                damage * lossRatio;

            if (lossAdded > 0.0f) {
                manager->AddLoss(lossAdded);
            }

            if (IsDebugLoggingEnabled()) {
                logs::info("================================");
                logs::info("Realtime Player Damage");
                logs::info("--------------------------------");
                logs::info(
                    "Previous HP:   {:.2f}",
                    previousHealth_);
                logs::info(
                    "Current HP:    {:.2f}",
                    currentHealth);
                logs::info(
                    "Damage:        {:.2f}",
                    damage);
                logs::info(
                    "Loss Ratio:    {:.2f}",
                    lossRatio);
                logs::info(
                    "Loss Added:    {:.2f}",
                    lossAdded);
                logs::info(
                    "Total Loss:    {:.2f}",
                    manager->GetLoss());
                logs::info(
                    "Recoverable:   {:.2f}",
                    manager->GetRecoverableHealth());
                logs::info("================================");
            }
        }


        // =========================================================
        // HEALING CAP
        //
        // Player HP may never remain above Recoverable HP.
        //
        // Large heals:
        //     Clamp immediately.
        //
        // Continuous/small heals:
        //     Clamp at a short interval.
        //
        // This preserves the existing tested behavior while avoiding
        // unnecessary ActorValue writes every frame.
        // =========================================================

        const float recoverableHealth =
            manager->GetRecoverableHealth();

        const float excessHealth =
            currentHealth - recoverableHealth;

        const bool exceedsCap =
            excessHealth > kDeltaEpsilon;

        const bool largeHeal =
            healthDelta > kLargeHealThreshold;

        bool healthWasClamped = false;


        if (exceedsCap) {

            // -----------------------------------------------------
            // Optional debug information
            // -----------------------------------------------------

            if (IsDebugLoggingEnabled()) {
                logs::info("================================");
                logs::info("Healing Cap");
                logs::info("--------------------------------");
                logs::info(
                    "Current HP:       {:.2f}",
                    currentHealth);
                logs::info(
                    "Recoverable HP:   {:.2f}",
                    recoverableHealth);
                logs::info(
                    "Excess HP:        {:.2f}",
                    excessHealth);
                logs::info(
                    "Health Delta:     {:.2f}",
                    healthDelta);
                logs::info(
                    "Clamp Cooldown:   {:.3f}",
                    clampCooldown_);
                logs::info(
                    "Large Heal:       {}",
                    largeHeal ? "true" : "false");
                logs::info("================================");
            }


            // -----------------------------------------------------
            // LARGE HEAL
            //
            // Potion, spell, food, etc. that causes a sufficiently
            // large positive HP delta.
            // -----------------------------------------------------

            if (largeHeal) {
                healthWasClamped =
                    manager->ClampCurrentHealth();

                clampCooldown_ =
                    kClampInterval;

                if (healthWasClamped &&
                    IsDebugLoggingEnabled()) {

                    logs::info(
                        "Large healing clamped immediately.");
                }
            }


            // -----------------------------------------------------
            // CONTINUOUS / SMALL HEAL
            //
            // Primarily handles passive health regeneration.
            // -----------------------------------------------------

            else if (clampCooldown_ <= 0.0f) {
                healthWasClamped =
                    manager->ClampCurrentHealth();

                if (healthWasClamped) {
                    clampCooldown_ =
                        kClampInterval;

                    if (IsDebugLoggingEnabled()) {
                        logs::info(
                            "Continuous healing clamped. "
                            "Next correction in {:.2f}s.",
                            kClampInterval);
                    }
                }
            }
        }


        // =========================================================
        // SYNCHRONIZE HEALTH BASELINE
        //
        // IMPORTANT:
        //
        // ClampCurrentHealth() damages Health internally to remove
        // HP above Recoverable HP.
        //
        // We must therefore synchronize previousHealth_ with the
        // FINAL post-clamp HP.
        //
        // Otherwise our own clamp could appear as player damage on
        // the next update and incorrectly create additional Loss.
        // =========================================================

        const float finalHealth =
            manager->GetCurrentHealth();

        if (std::isfinite(finalHealth)) {

            if (healthWasClamped &&
                IsDebugLoggingEnabled()) {

                logs::info(
                    "Healing clamp baseline synchronized: {:.2f}",
                    finalHealth);
            }

            previousHealth_ =
                finalHealth;
        }
        else {
            previousHealth_ =
                currentHealth;
        }
    }


    void PlayerUpdateHook::ResetHealthSnapshot()
    {
        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            previousHealth_ = 0.0f;
            initialized_ = false;
            clampCooldown_ = 0.0f;

            if (IsDebugLoggingEnabled()) {
                logs::info(
                    "Realtime health snapshot reset: "
                    "player unavailable.");
            }

            return;
        }


        auto* actorValueOwner =
            player->AsActorValueOwner();

        if (!actorValueOwner) {
            previousHealth_ = 0.0f;
            initialized_ = false;
            clampCooldown_ = 0.0f;

            if (IsDebugLoggingEnabled()) {
                logs::info(
                    "Realtime health snapshot reset: "
                    "ActorValueOwner unavailable.");
            }

            return;
        }


        const float currentHealth =
            actorValueOwner->GetActorValue(
                RE::ActorValue::kHealth);

        if (!std::isfinite(currentHealth)) {
            previousHealth_ = 0.0f;
            initialized_ = false;
            clampCooldown_ = 0.0f;

            if (IsDebugLoggingEnabled()) {
                logs::info(
                    "Realtime health snapshot reset: "
                    "invalid Health value.");
            }

            return;
        }


        previousHealth_ =
            currentHealth;

        initialized_ =
            true;

        clampCooldown_ =
            0.0f;


        if (IsDebugLoggingEnabled()) {
            logs::info(
                "Realtime health snapshot reset: {:.2f}",
                previousHealth_);
        }
    }


    float PlayerUpdateHook::GetLastGameHours()
    {
        if (!gameTimeInitialized_) {

            if (auto* calendar =
                    RE::Calendar::GetSingleton()) {

                const float currentGameHours =
                    calendar->GetHoursPassed();

                if (std::isfinite(currentGameHours)) {
                    lastGameHours_ =
                        currentGameHours;

                    gameTimeInitialized_ =
                        true;
                }
            }
        }

        return lastGameHours_;
    }


    void PlayerUpdateHook::ResetGameTimeSnapshot()
    {
        auto* calendar =
            RE::Calendar::GetSingleton();

        if (!calendar) {
            lastGameHours_ = 0.0f;
            gameTimeInitialized_ = false;

            if (IsDebugLoggingEnabled()) {
                logs::info(
                    "Game-time snapshot reset: "
                    "Calendar unavailable.");
            }

            return;
        }


        const float currentGameHours =
            calendar->GetHoursPassed();

        if (!std::isfinite(currentGameHours)) {
            lastGameHours_ = 0.0f;
            gameTimeInitialized_ = false;

            if (IsDebugLoggingEnabled()) {
                logs::info(
                    "Game-time snapshot reset: "
                    "invalid Calendar value.");
            }

            return;
        }


        lastGameHours_ =
            currentGameHours;

        gameTimeInitialized_ =
            true;


        if (IsDebugLoggingEnabled()) {
            logs::info(
                "Game-time snapshot reset: {:.4f}",
                lastGameHours_);
        }
    }
}