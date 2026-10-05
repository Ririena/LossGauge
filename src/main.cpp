#include "pch.h"

#include "Config/ConfigManager.h"
#include "Events/SleepEventHandler.h"
#include "Gameplay/LossManager.h"
#include "Hooks/PlayerUpdateHook.h"
#include "Serialization/Serialization.h"

namespace
{
    void InitializeGameplay()
    {
        logs::info(
            "Initializing Loss Gauge gameplay...");

        auto* player =
            RE::PlayerCharacter::GetSingleton();

        if (!player) {
            logs::warn(
                "PlayerCharacter is null.");
            return;
        }

        logs::info(
            "Player found. FormID: {:08X}",
            player->GetFormID());

        auto* actorValueOwner =
            player->AsActorValueOwner();

        if (!actorValueOwner) {
            logs::critical(
                "ActorValueOwner is null.");
            return;
        }

        logs::info(
            "ActorValueOwner acquired successfully.");

        auto* manager =
            LossGauge::LossManager::GetSingleton();

        const float currentHealth =
            manager->GetCurrentHealth();

        const float permanentHealth =
            manager->GetPermanentHealth();

        const float maxHealth =
            manager->GetMaxHealth();

        const float loss =
            manager->GetLoss();

        const float recoverableHealth =
            manager->GetRecoverableHealth();

        logs::info("================================");
        logs::info("Loss Gauge Health State");
        logs::info("--------------------------------");

        logs::info(
            "Current HP:     {:.2f}",
            currentHealth);

        logs::info(
            "Permanent HP:   {:.2f}",
            permanentHealth);

        logs::info(
            "Max HP:         {:.2f}",
            maxHealth);

        logs::info(
            "Loss:           {:.2f}",
            loss);

        logs::info(
            "Recoverable HP: {:.2f}",
            recoverableHealth);

        logs::info("================================");

        LossGauge::PlayerUpdateHook::
            ResetHealthSnapshot();

        logs::info(
            "Loss Gauge gameplay "
            "initialized successfully.");
    }

    void MessageHandler(
        SKSE::MessagingInterface::Message* a_message)
    {
        if (!a_message) {
            return;
        }

        switch (a_message->type) {

        case SKSE::MessagingInterface::kPostLoad:
        {
            logs::info(
                "SKSE message: PostLoad");
            break;
        }

        case SKSE::MessagingInterface::kPostPostLoad:
        {
            logs::info(
                "SKSE message: PostPostLoad");
            break;
        }

        case SKSE::MessagingInterface::kDataLoaded:
        {
            logs::info(
                "SKSE message: DataLoaded");

            LossGauge::SleepEventHandler::
                Register();

            break;
        }

        case SKSE::MessagingInterface::kNewGame:
        {
            logs::info(
                "SKSE message: NewGame");

            LossGauge::LossManager::
                GetSingleton()->
                ResetLoss();

            InitializeGameplay();

            break;
        }

        case SKSE::MessagingInterface::kPostLoadGame:
        {
            logs::info(
                "SKSE message: PostLoadGame");

            //!IMPORTANT:
            // Do not reset Loss here.
            //
            // Loss is save-specific and is restored
            // by SKSE Serialization::LoadCallback.

            InitializeGameplay();

            break;
        }

        default:
            break;
        }
    }
}

SKSE_PLUGIN_LOAD(
    const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(
        a_skse);

    logs::info("================================");
    logs::info("Loss Gauge");
    logs::info("Version 0.1.0");
    logs::info("================================");

    // Configuration

    auto* config =
        LossGauge::ConfigManager::
            GetSingleton();

    config->Load();

    // Serialization

    LossGauge::Serialization::
        Register();

    // Gameplay hooks

    LossGauge::PlayerUpdateHook::
        Install();

    // SKSE Lggoger

    auto* messaging =
        SKSE::GetMessagingInterface();

    if (!messaging) {
        logs::critical(
            "Failed to get "
            "SKSE Messaging Interface.");

        return false;
    }

    if (!messaging->RegisterListener(
            MessageHandler)) {

        logs::critical(
            "Failed to register "
            "SKSE messaging listener.");

        return false;
    }

    logs::info(
        "SKSE messaging listener registered.");

    logs::info(
        "Loss Gauge loaded successfully.");

    return true;
}