#include "Gameplay/NaturalRegenController.h"

#include "Config/ConfigManager.h"

namespace LossGauge
{
    NaturalRegenController*
    NaturalRegenController::GetSingleton()
    {
        static NaturalRegenController instance;
        return std::addressof(instance);
    }

    void NaturalRegenController::Apply()
    {
        auto* config =
            ConfigManager::GetSingleton();

        if (!config) {
            logs::warn(
                "NaturalRegenController: "
                "ConfigManager is null.");

            return;
        }

        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            logs::warn(
                "NaturalRegenController: "
                "PlayerCharacter is null.");

            return;
        }

        auto* actorValueOwner =
            player->AsActorValueOwner();

        if (!actorValueOwner) {
            logs::warn(
                "NaturalRegenController: "
                "ActorValueOwner is null.");

            return;
        }

        // ========================================
        // Natural regeneration enabled
        // ========================================

        if (config->
                IsNaturalHealthRegenerationEnabled()) {

            // If Loss Gauge applied a temporary
            // modifier during this runtime session,
            // remove it.
            Restore();

            const float currentHealRateMult =
                actorValueOwner->
                    GetActorValue(
                        RE::ActorValue::
                            kHealRateMult);

            logs::info(
                "================================");

            logs::info(
                "Natural Health Regeneration");

            logs::info(
                "--------------------------------");

            logs::info(
                "Natural HP Regen: Enabled");

            logs::info(
                "HealRateMult: {:.2f}",
                currentHealRateMult);

            logs::info(
                "================================");

            return;
        }

        // ========================================
        // Already suppressed
        // ========================================

        if (suppressionApplied_) {
            return;
        }

        // ========================================
        // Read current effective HealRateMult
        // ========================================

        const float currentHealRateMult =
            actorValueOwner->
                GetActorValue(
                    RE::ActorValue::
                        kHealRateMult);

        if (!std::isfinite(
                currentHealRateMult)) {

            logs::warn(
                "NaturalRegenController: "
                "HealRateMult is invalid.");

            return;
        }

        logs::info(
            "================================");

        logs::info(
            "Natural Health Regeneration");

        logs::info(
            "--------------------------------");

        logs::info(
            "HealRateMult Before: {:.2f}",
            currentHealRateMult);

        // ========================================
        // Apply suppression
        // ========================================
        //
        // Do NOT use:
        //
        // SetActorValue(kHealRateMult, 0.0f)
        //
        // because SetActorValue can alter the
        // ActorValue state that Skyrim saves.
        //
        // Instead we apply a temporary modifier.
        //
        // Example:
        //
        // HealRateMult = 100
        // modifier     = -100
        //
        // effective HealRateMult = 0
        //
        // We remember exactly how much we applied
        // so Restore() can reverse it.

        appliedModifier_ =
            -currentHealRateMult;

        if (std::abs(
                appliedModifier_) >
            0.001f) {

            actorValueOwner->
                ModActorValue(
                    RE::ACTOR_VALUE_MODIFIER::
                        kTemporary,
                    RE::ActorValue::
                        kHealRateMult,
                    appliedModifier_);
        }

        const float after =
            actorValueOwner->
                GetActorValue(
                    RE::ActorValue::
                        kHealRateMult);

        suppressionApplied_ =
            true;

        logs::info(
            "Applied Modifier:    {:.2f}",
            appliedModifier_);

        logs::info(
            "HealRateMult After:  {:.2f}",
            after);

        logs::info(
            "Natural HP Regen:    Disabled");

        logs::info(
            "================================");
    }

    void NaturalRegenController::Restore()
    {
        // Nothing was applied by Loss Gauge
        // during this runtime session.
        if (!suppressionApplied_) {
            return;
        }

        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            logs::warn(
                "NaturalRegenController: "
                "PlayerCharacter is null "
                "during Restore().");

            return;
        }

        auto* actorValueOwner =
            player->AsActorValueOwner();

        if (!actorValueOwner) {
            logs::warn(
                "NaturalRegenController: "
                "ActorValueOwner is null "
                "during Restore().");

            return;
        }

        const float before =
            actorValueOwner->
                GetActorValue(
                    RE::ActorValue::
                        kHealRateMult);

        // ========================================
        // Remove our temporary modifier
        // ========================================
        //
        // Example:
        //
        // Apply:
        //     -100
        //
        // Restore:
        //     +100

        const float restoreModifier =
            -appliedModifier_;

        if (std::abs(
                restoreModifier) >
            0.001f) {

            actorValueOwner->
                ModActorValue(
                    RE::ACTOR_VALUE_MODIFIER::
                        kTemporary,
                    RE::ActorValue::
                        kHealRateMult,
                    restoreModifier);
        }

        const float after =
            actorValueOwner->
                GetActorValue(
                    RE::ActorValue::
                        kHealRateMult);

        logs::info(
            "================================");

        logs::info(
            "Natural Health Regeneration Restore");

        logs::info(
            "--------------------------------");

        logs::info(
            "HealRateMult Before: {:.2f}",
            before);

        logs::info(
            "Removed Modifier:    {:.2f}",
            restoreModifier);

        logs::info(
            "HealRateMult After:  {:.2f}",
            after);

        logs::info(
            "================================");

        ResetState();
    }

    void NaturalRegenController::ResetState()
    {
        // Runtime bookkeeping only.
        //
        // IMPORTANT:
        // This function intentionally does NOT
        // modify any Skyrim ActorValue.

        suppressionApplied_ =
            false;

        appliedModifier_ =
            0.0f;
    }

    bool NaturalRegenController::
    IsSuppressionApplied() const
    {
        return suppressionApplied_;
    }
}