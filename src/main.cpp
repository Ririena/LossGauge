#include "pch.h"

#include "Config/ConfigManager.h"
#include "Events/SleepEventHandler.h"
#include "Gameplay/LossManager.h"
#include "Gameplay/NaturalRegenController.h"
#include "Hooks/PlayerUpdateHook.h"
#include "Menu/MenuManager.h"
#include "Serialization/Serialization.h"
#include "UI/PrismaUIBridge.h"
#include "UI/PrismaUIEditor.h"
#include "UI/UIStateManager.h"

#include "External/PrismaUI_API.h"

namespace
{
    // PrismaUI

    PRISMA_UI_API::IVPrismaUI1*
        g_prismaUI = nullptr;

    // HUD view:
    // LossGauge/index.html
    PrismaView
        g_prismaView = 0;

    // Editor view:
    // LossGauge/editor.html
    PrismaView
        g_prismaEditorView = 0;



    void OnPrismaDomReady(
        PrismaView a_view)
    {
        logs::info(
            "PrismaUI DOM ready. View: {}",
            a_view);

        if (!g_prismaUI) {
            logs::error(
                "PrismaUI API is null "
                "inside DOM ready callback.");

            return;
        }

        if (!g_prismaUI->IsValid(
                a_view)) {

            logs::error(
                "PrismaUI view is invalid "
                "inside DOM ready callback.");

            return;
        }

        g_prismaView =
            a_view;

        auto* bridge =
            LossGauge::
                PrismaUIBridge::
                    GetSingleton();

        bridge->Initialize(
            g_prismaUI,
            g_prismaView);

        bridge->SetDomReady(
            true);

        // Force a fresh UI state
        auto* uiStateManager =
            LossGauge::
                UIStateManager::
                    GetSingleton();

        uiStateManager->Reset();

        if (uiStateManager->Update()) {
            (void)bridge->SendState(
                uiStateManager->
                    GetState());
        }

        logs::info(
            "Loss Gauge PrismaUI "
            "realtime bridge ready.");
    }


    // PrismaUI Editor DOM Ready

    void OnPrismaEditorDomReady(
        PrismaView a_view)
    {
        logs::info(
            "PrismaUI Editor DOM ready. "
            "View: {}",
            a_view);

        if (!g_prismaUI) {
            logs::error(
                "PrismaUI API is null "
                "inside Editor DOM ready "
                "callback.");

            return;
        }

        if (!g_prismaUI->IsValid(
                a_view)) {

            logs::error(
                "PrismaUI Editor view is "
                "invalid inside DOM ready "
                "callback.");

            return;
        }

        g_prismaEditorView =
            a_view;

        auto* editor =
            LossGauge::
                PrismaUIEditor::
                    GetSingleton();

        editor->Initialize(
            g_prismaUI,
            g_prismaEditorView);

        editor->SetDomReady(
            true);

        logs::info(
            "Loss Gauge PrismaUI "
            "Editor ready.");
    }


    // ========================================
    // Gameplay Initialization
    // ========================================

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
            player->
                AsActorValueOwner();

        if (!actorValueOwner) {
            logs::critical(
                "ActorValueOwner is null.");

            return;
        }

        logs::info(
            "ActorValueOwner acquired "
            "successfully.");


        // ========================================
        // Natural Health Regeneration
        // ========================================

        LossGauge::
            NaturalRegenController::
                GetSingleton()->
                    Apply();


        // ========================================
        // Loss Gauge
        // ========================================

        auto* manager =
            LossGauge::
                LossManager::
                    GetSingleton();

        const float currentHealth =
            manager->
                GetCurrentHealth();

        const float permanentHealth =
            manager->
                GetPermanentHealth();

        const float maxHealth =
            manager->
                GetMaxHealth();

        const float loss =
            manager->
                GetLoss();

        const float recoverableHealth =
            manager->
                GetRecoverableHealth();

        logs::info(
            "================================");

        logs::info(
            "Loss Gauge Health State");

        logs::info(
            "--------------------------------");

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

        logs::info(
            "================================");


        // ========================================
        // Synchronize Realtime Snapshots
        // ========================================

        LossGauge::
            PlayerUpdateHook::
                ResetHealthSnapshot();

        LossGauge::
            PlayerUpdateHook::
                ResetGameTimeSnapshot();


        // ========================================
        // UI State
        // ========================================

        // PrismaUI is now the only runtime
        // UI path used by Loss Gauge.
        LossGauge::
            UIStateManager::
                GetSingleton()->
                    Reset();

        logs::info(
            "Loss Gauge gameplay "
            "initialized successfully.");
    }


    // ========================================
    // SKSE Messaging
    // ========================================

    void MessageHandler(
        SKSE::MessagingInterface::Message*
            a_message)
    {
        if (!a_message) {
            return;
        }

        switch (a_message->type) {

        // ========================================
        // PostLoad
        // ========================================

        case SKSE::MessagingInterface::
            kPostLoad:
        {
            logs::info(
                "SKSE message: PostLoad");

            break;
        }


        // ========================================
        // PostPostLoad
        // ========================================

        case SKSE::MessagingInterface::
            kPostPostLoad:
        {
            logs::info(
                "SKSE message: PostPostLoad");

            LossGauge::
                MenuManager::
                    Register();

            break;
        }


        // ========================================
        // DataLoaded
        // ========================================

        case SKSE::MessagingInterface::
            kDataLoaded:
        {
            logs::info(
                "SKSE message: DataLoaded");


            // ====================================
            // Sleep Event Handler
            // ====================================

            LossGauge::
                SleepEventHandler::
                    Register();


            // ====================================
            // PrismaUI API
            // ====================================

            g_prismaUI =
                static_cast<
                    PRISMA_UI_API::
                        IVPrismaUI1*>(
                    PRISMA_UI_API::
                        RequestPluginAPI(
                            PRISMA_UI_API::
                                InterfaceVersion::
                                    V1));

            if (!g_prismaUI) {
                logs::error(
                    "PrismaUI API V1 unavailable.");

                break;
            }

            logs::info(
                "PrismaUI API V1 acquired.");


            // ====================================
            // Create PrismaUI HUD View
            // ====================================

            logs::info(
                "Creating Loss Gauge "
                "PrismaUI view...");

            g_prismaView =
                g_prismaUI->
                    CreateView(
                        "LossGauge/index.html",
                        OnPrismaDomReady);

            if (g_prismaView == 0) {
                logs::error(
                    "Failed to create "
                    "Loss Gauge PrismaUI view.");

                break;
            }

            logs::info(
                "PrismaUI view created. "
                "View: {}",
                g_prismaView);


            // ====================================
            // Create PrismaUI Editor View
            // ====================================

            logs::info(
                "Creating Loss Gauge "
                "PrismaUI Editor view...");

           g_prismaEditorView =
    g_prismaUI->
        CreateView(
            "LossGauge/editor.html",
            OnPrismaEditorDomReady);

if (g_prismaEditorView == 0) {
    logs::error(
        "Failed to create "
        "Loss Gauge PrismaUI "
        "Editor view.");

    break;
}



if (g_prismaUI->IsValid(
        g_prismaEditorView)) {

    g_prismaUI->Hide(
        g_prismaEditorView);
}

logs::info(
    "PrismaUI Editor view created "
    "and hidden. View: {}",
    g_prismaEditorView);
        }


        // ========================================
        // New Game
        // ========================================

        case SKSE::MessagingInterface::
            kNewGame:
        {
            logs::info(
                "SKSE message: NewGame");

            LossGauge::
                LossManager::
                    GetSingleton()->
                        ResetLoss();

            InitializeGameplay();

            break;
        }


        // ========================================
        // Post Load Game
        // ========================================

        case SKSE::MessagingInterface::
            kPostLoadGame:
        {
            logs::info(
                "SKSE message: PostLoadGame");

            // Do NOT reset Loss here.
            //
            // Serialization has already restored
            // the save-specific Loss state.
            InitializeGameplay();

            break;
        }


        default:
            break;
        }
    }
}


// ============================================
// SKSE Plugin Entry Point
// ============================================

SKSE_PLUGIN_LOAD(
    const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(
        a_skse);

    logs::info(
        "================================");

    logs::info(
        "Loss Gauge");

    logs::info(
        "Version 0.1.0");

    logs::info(
        "================================");


    // ========================================
    // Configuration
    // ========================================

    auto* config =
        LossGauge::
            ConfigManager::
                GetSingleton();

    config->Load();


    // ========================================
    // Serialization
    // ========================================

    LossGauge::
        Serialization::
            Register();


    // ========================================
    // Player Update Hook
    // ========================================

    LossGauge::
        PlayerUpdateHook::
            Install();


    // ========================================
    // SKSE Messaging
    // ========================================

    auto* messaging =
        SKSE::
            GetMessagingInterface();

    if (!messaging) {
        logs::critical(
            "Failed to get "
            "SKSE Messaging Interface.");

        return false;
    }

    if (!messaging->
            RegisterListener(
                MessageHandler)) {

        logs::critical(
            "Failed to register "
            "SKSE messaging listener.");

        return false;
    }

    logs::info(
        "SKSE messaging listener "
        "registered.");

    logs::info(
        "Loss Gauge loaded successfully.");

    return true;
}