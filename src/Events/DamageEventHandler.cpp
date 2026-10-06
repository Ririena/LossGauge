#include "Events/DamageEventHandler.h"

#include "Gameplay/LossManager.h"


namespace LossGauge
{
    namespace
    {
        // Prototype:
        // 10% of HP damage becomes Loss.
        constexpr float kLossRatio = 0.10f;
    }


    DamageEventHandler*
    DamageEventHandler::GetSingleton()
    {
        static DamageEventHandler instance;

        return std::addressof(instance);
    }


    void DamageEventHandler::Register()
    {
        // Prevent duplicate registration.
        if (registered_) {
            logs::info(
                "DamageEventHandler already registered.");

            return;
        }


        auto* eventSource =
            RE::ScriptEventSourceHolder::GetSingleton();

        if (!eventSource) {
            logs::critical(
                "DamageEventHandler: "
                "ScriptEventSourceHolder is null.");

            return;
        }


        eventSource->AddEventSink<RE::TESHitEvent>(
            this);


        registered_ = true;


        logs::info(
            "DamageEventHandler registered.");
    }


    void DamageEventHandler::ResetHealthSnapshot()
    {
        auto* manager =
            LossManager::GetSingleton();


        previousHealth_ =
            manager->GetCurrentHealth();


        initialized_ =
            previousHealth_ > 0.0f;


        logs::info(
            "Damage health snapshot: {:.2f}",
            previousHealth_);
    }


    RE::BSEventNotifyControl
    DamageEventHandler::ProcessEvent(
        const RE::TESHitEvent* a_event,
        RE::BSTEventSource<RE::TESHitEvent>*)
    {
        if (!a_event) {
            return
                RE::BSEventNotifyControl::kContinue;
        }


        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            return
                RE::BSEventNotifyControl::kContinue;
        }


        // Get the object that received the hit.
        auto* target =
            a_event->target.get();

        if (!target) {
            return
                RE::BSEventNotifyControl::kContinue;
        }


        // Ignore hits against NPCs / enemies.
        if (target != player) {
            return
                RE::BSEventNotifyControl::kContinue;
        }


        auto* manager =
            LossManager::GetSingleton();


        const float currentHealth =
            manager->GetCurrentHealth();


        if (!initialized_) {

            previousHealth_ =
                currentHealth;

            initialized_ = true;


            logs::info(
                "Damage tracker baseline initialized: {:.2f}",
                currentHealth);


            return
                RE::BSEventNotifyControl::kContinue;
        }


        // Calculate actual HP difference.
        const float actualDamage =
            previousHealth_ - currentHealth;


        logs::info(
            "TESHitEvent received for player.");

        logs::info(
            "Previous HP: {:.2f} | Current HP: {:.2f}",
            previousHealth_,
            currentHealth);


        // No HP was lost.
        if (actualDamage <= 0.0f) {

            logs::info(
                "Hit produced no detectable HP damage.");

            previousHealth_ =
                currentHealth;


            return
                RE::BSEventNotifyControl::kContinue;
        }


        // Calculate Loss.
        const float lossGained =
            actualDamage * kLossRatio;


        // Add Loss to our manager.
        manager->AddLoss(
            lossGained);


        const float maxHealth =
            manager->GetMaxHealth();


        const float recoverableHealth =
            manager->GetRecoverableHealth();


        // Debug output.
        logs::info(
            "================================");

        logs::info(
            "Player Damage Detected");

        logs::info(
            "--------------------------------");


        logs::info(
            "Damage:         {:.2f}",
            actualDamage);


        logs::info(
            "Loss Ratio:     {:.0f}%",
            kLossRatio * 100.0f);


        logs::info(
            "Loss Gained:    {:.2f}",
            lossGained);


        logs::info(
            "Total Loss:     {:.2f}",
            manager->GetLoss());


        logs::info(
            "Current HP:     {:.2f}",
            currentHealth);


        logs::info(
            "Max HP:         {:.2f}",
            maxHealth);


        logs::info(
            "Recoverable HP: {:.2f}",
            recoverableHealth);


        logs::info(
            "================================");


        // Current HP becomes baseline for next hit.
        previousHealth_ =
            currentHealth;


        return
            RE::BSEventNotifyControl::kContinue;
    }
}