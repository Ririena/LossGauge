#include "Hooks/PlayerUpdateHook.h"

#include "Config/ConfigManager.h"
#include "Gameplay/LossManager.h"
#include "UI/PrismaUIBridge.h"
#include "UI/UIStateManager.h"

namespace LossGauge
{
    namespace
    {
        constexpr float kDeltaEpsilon =
            0.001f;

        constexpr float kLargeHealThreshold =
            1.0f;

        constexpr float kClampInterval =
            0.10f;


        // ========================================
        // Debug Logging
        // ========================================

        bool IsDebugLoggingEnabled()
        {
            const auto* config =
                ConfigManager::
                    GetSingleton();

            return
                config &&
                config->
                    IsDebugLoggingEnabled();
        }


        // ========================================
        // UI State
        // ========================================

        void UpdateUIState()
        {
            auto* uiStateManager =
                UIStateManager::
                    GetSingleton();

            if (!uiStateManager) {
                return;
            }

            // Only transmit when the visible
            // gameplay state actually changed.
            if (!uiStateManager->
                    Update()) {

                return;
            }

            const auto& state =
                uiStateManager->
                    GetState();


            // ====================================
            // PrismaUI Bridge
            // ====================================

            auto* prismaBridge =
                PrismaUIBridge::
                    GetSingleton();

            if (prismaBridge) {
                (void)prismaBridge->
                    SendState(
                        state);
            }
        }
    }


    // ========================================
    // Install
    // ========================================

    void PlayerUpdateHook::Install()
    {
        REL::Relocation<
            std::uintptr_t>
            vtable{
                RE::PlayerCharacter::
                    VTABLE[0]
            };

        originalUpdate_ =
            vtable.write_vfunc(
                0xAD,
                Update);

        logs::info(
            "PlayerUpdateHook installed.");
    }


    // ========================================
    // Player Update
    // ========================================

    void PlayerUpdateHook::Update(
        RE::PlayerCharacter* a_player,
        float a_delta)
    {
        // Always let Skyrim update first.
        originalUpdate_(
            a_player,
            a_delta);

        if (!a_player) {
            return;
        }

        auto* manager =
            LossManager::
                GetSingleton();

        auto* config =
            ConfigManager::
                GetSingleton();

        if (!manager ||
            !config) {

            return;
        }

        auto* actorValueOwner =
            a_player->
                AsActorValueOwner();

        if (!actorValueOwner) {
            return;
        }


        // ========================================
        // Game-Time Snapshot
        // ========================================

        if (auto* calendar =
                RE::Calendar::
                    GetSingleton()) {

            const float currentGameHours =
                calendar->
                    GetHoursPassed();

            if (std::isfinite(
                    currentGameHours)) {

                lastGameHours_ =
                    currentGameHours;

                gameTimeInitialized_ =
                    true;
            }
        }


        // ========================================
        // Current Health
        // ========================================

        const float currentHealth =
            actorValueOwner->
                GetActorValue(
                    RE::ActorValue::
                        kHealth);

        if (!std::isfinite(
                currentHealth)) {

            return;
        }


        // ========================================
        // First Health Snapshot
        // ========================================

        if (!initialized_) {
            previousHealth_ =
                currentHealth;

            initialized_ =
                true;

            clampCooldown_ =
                0.0f;

            if (IsDebugLoggingEnabled()) {
                logs::info(
                    "PlayerUpdateHook health "
                    "baseline initialized: {:.2f}",
                    previousHealth_);
            }

            UpdateUIState();

            return;
        }


        // ========================================
        // Clamp Cooldown
        // ========================================

        if (std::isfinite(a_delta) &&
            a_delta > 0.0f) {

            clampCooldown_ =
                (std::max)(
                    0.0f,
                    clampCooldown_ -
                        a_delta);
        }

        const float healthDelta =
            currentHealth -
            previousHealth_;


        // ========================================
        // Damage Detection
        // ========================================

        if (healthDelta <
            -kDeltaEpsilon) {

            const float damage =
                -healthDelta;

            const float lossRatio =
                config->
                    GetLossRatio();

            const float lossAdded =
                damage *
                lossRatio;

            if (lossAdded > 0.0f) {
                manager->
                    AddLoss(
                        lossAdded);
            }

            if (IsDebugLoggingEnabled()) {
                logs::info(
                    "Damage detected: {:.2f}",
                    damage);

                logs::info(
                    "Loss Ratio: {:.2f}",
                    lossRatio);

                logs::info(
                    "Loss Added: {:.2f}",
                    lossAdded);
            }
        }


        // ========================================
        // Recoverable Health Clamp
        // ========================================

        const float recoverableHealth =
            manager->
                GetRecoverableHealth();

        const float excessHealth =
            currentHealth -
            recoverableHealth;

        const bool exceedsCap =
            excessHealth >
            kDeltaEpsilon;

        const bool largeHeal =
            healthDelta >
            kLargeHealThreshold;

        bool healthWasClamped =
            false;

        if (exceedsCap) {
            if (IsDebugLoggingEnabled()) {
                logs::info(
                    "Healing exceeded "
                    "recoverable HP.");

                logs::info(
                    "Current HP: {:.2f}",
                    currentHealth);

                logs::info(
                    "Recoverable HP: {:.2f}",
                    recoverableHealth);

                logs::info(
                    "Excess HP: {:.2f}",
                    excessHealth);
            }

            // Large healing events are clamped
            // immediately.
            if (largeHeal) {
                healthWasClamped =
                    manager->
                        ClampCurrentHealth();

                clampCooldown_ =
                    kClampInterval;
            }

            // Continuous regeneration is corrected
            // at a controlled interval.
            else if (
                clampCooldown_ <= 0.0f) {

                healthWasClamped =
                    manager->
                        ClampCurrentHealth();

                if (healthWasClamped) {
                    clampCooldown_ =
                        kClampInterval;
                }
            }
        }

        if (healthWasClamped &&
            IsDebugLoggingEnabled()) {

            logs::info(
                "Healing clamped to "
                "recoverable HP.");
        }


        // ========================================
        // Synchronize Health Baseline
        // ========================================

        const float finalHealth =
            manager->
                GetCurrentHealth();

        if (std::isfinite(
                finalHealth)) {

            previousHealth_ =
                finalHealth;
        }
        else {
            previousHealth_ =
                currentHealth;
        }


        // ========================================
        // UI State
        // ========================================
        //
        // UIStateManager decides whether the
        // visible state actually changed.
        //
        // PrismaUIBridge is the only runtime
        // UI output path.

        UpdateUIState();
    }


    // ========================================
    // Reset Health Snapshot
    // ========================================

    void PlayerUpdateHook::
        ResetHealthSnapshot()
    {
        previousHealth_ =
            0.0f;

        initialized_ =
            false;

        clampCooldown_ =
            0.0f;

        if (IsDebugLoggingEnabled()) {
            logs::info(
                "PlayerUpdateHook health "
                "snapshot reset.");
        }
    }


    // ========================================
    // Get Last Game Hours
    // ========================================

    float PlayerUpdateHook::
        GetLastGameHours()
    {
        if (!gameTimeInitialized_) {
            return 0.0f;
        }

        return lastGameHours_;
    }


    // ========================================
    // Reset Game-Time Snapshot
    // ========================================

    void PlayerUpdateHook::
        ResetGameTimeSnapshot()
    {
        lastGameHours_ =
            0.0f;

        gameTimeInitialized_ =
            false;

        if (IsDebugLoggingEnabled()) {
            logs::info(
                "PlayerUpdateHook game-time "
                "snapshot reset.");
        }
    }
}