#include "Menu/MenuManager.h"

#include "Config/ConfigManager.h"
#include "External/SKSEMenuFramework.h"
#include "Gameplay/LossManager.h"
#include "Gameplay/NaturalRegenController.h"
#include "UI/PrismaUIEditor.h"

#include <algorithm>
#include <cstdio>

namespace LossGauge
{
    bool MenuManager::registered_ = false;

    // Pending settings
    bool MenuManager::pendingInitialized_ = false;

    float MenuManager::pendingLossRatio_ = 0.10f;

    bool MenuManager::pendingSleepRecovery_ = true;
    float MenuManager::pendingFullRecoveryHours_ = 8.0f;

    bool MenuManager::pendingNaturalRegen_ = false;


    // Register

    void MenuManager::Register()
    {
        if (registered_) {
            return;
        }

        if (!SKSEMenuFramework::IsInstalled()) {
            logs::info(
                "SKSE Menu Framework not detected. "
                "Menu integration disabled.");

            return;
        }

        const float frameworkVersion =
            SKSEMenuFramework::
                GetMenuFrameworkVersion();

        const std::uint32_t apiVersion =
            SKSEMenuFramework::
                GetMenuFrameworkAPIVersion();

        logs::info(
            "SKSE Menu Framework detected.");

        logs::info(
            "Framework Version: {:.2f}",
            frameworkVersion);

        logs::info(
            "Framework API Version: {}",
            apiVersion);

        SKSEMenuFramework::
            SetSection(
                "Loss Gauge");

        SKSEMenuFramework::
            AddSectionItem(
                "Settings",
                RenderSettings);

        SKSEMenuFramework::
            AddSectionItem(
                "Debug",
                RenderDebug);

        registered_ = true;

        logs::info(
            "Loss Gauge menu registered.");
    }


    // Pending settings

    void MenuManager::InitializePendingSettings()
    {
        auto* config =
            ConfigManager::
                GetSingleton();

        if (!config) {
            return;
        }

        pendingLossRatio_ =
            config->
                GetLossRatio();

        pendingSleepRecovery_ =
            config->
                IsSleepRecoveryEnabled();

        pendingFullRecoveryHours_ =
            config->
                GetFullRecoveryHours();

        pendingNaturalRegen_ =
            config->
                IsNaturalHealthRegenerationEnabled();

        pendingInitialized_ = true;
    }


    void MenuManager::ResetPendingSettings()
    {
        // Match ConfigManager defaults.

        pendingLossRatio_ =
            0.10f;

        pendingSleepRecovery_ =
            true;

        pendingFullRecoveryHours_ =
            8.0f;

        pendingNaturalRegen_ =
            false;
    }


    // Settings

    void __stdcall MenuManager::RenderSettings()
    {
        auto* config =
            ConfigManager::
                GetSingleton();

        if (!config) {
            ImGuiMCP::Text(
                "Loss Gauge configuration "
                "is unavailable.");

            return;
        }

        // Load active config once into the menu working copy.

        if (!pendingInitialized_) {
            InitializePendingSettings();
        }


        // Loss

        ImGuiMCP::Text(
            "Loss");

        ImGuiMCP::Separator();

        ImGuiMCP::SliderFloat(
            "Loss Ratio",
            &pendingLossRatio_,
            0.0f,
            1.0f,
            "%.2f");

        pendingLossRatio_ =
            std::clamp(
                pendingLossRatio_,
                0.0f,
                1.0f);

        ImGuiMCP::Text(
            "Percentage of incoming damage "
            "converted into Loss.");

        ImGuiMCP::Spacing();


        // Sleep Recovery

        ImGuiMCP::Text(
            "Sleep Recovery");

        ImGuiMCP::Separator();

        ImGuiMCP::Checkbox(
            "Enable Sleep Recovery",
            &pendingSleepRecovery_);

        ImGuiMCP::SliderFloat(
            "Full Recovery Hours",
            &pendingFullRecoveryHours_,
            1.0f,
            24.0f,
            "%.1f h");

        pendingFullRecoveryHours_ =
            std::clamp(
                pendingFullRecoveryHours_,
                1.0f,
                24.0f);

        ImGuiMCP::Text(
            "Cumulative sleep time required "
            "to fully recover Loss.");

        ImGuiMCP::Spacing();


        // Health

        ImGuiMCP::Text(
            "Health");

        ImGuiMCP::Separator();

        ImGuiMCP::Checkbox(
            "Natural Health Regeneration",
            &pendingNaturalRegen_);

        ImGuiMCP::Text(
            "Controls Skyrim's natural "
            "health regeneration.");

        ImGuiMCP::Spacing();


        // User Interface

        ImGuiMCP::Text(
            "User Interface");

        ImGuiMCP::Separator();

        ImGuiMCP::Text(
            "Customize the Loss Gauge HUD "
            "using the PrismaUI editor.");

        ImGuiMCP::Spacing();

        auto* editor =
            PrismaUIEditor::
                GetSingleton();

        if (!editor) {
            ImGuiMCP::Text(
                "PrismaUI Editor is "
                "unavailable.");
        }
        else if (!editor->
                     IsInitialized()) {

            ImGuiMCP::Text(
                "PrismaUI Editor is not "
                "initialized yet.");
        }
        else if (!editor->
                     IsDomReady()) {

            ImGuiMCP::Text(
                "PrismaUI Editor is "
                "still loading.");
        }
        else if (editor->
                     IsOpen()) {

            ImGuiMCP::Text(
                "UI Editor is currently open.");
        }
        else {
            if (ImGuiMCP::Button(
                    "Open UI Editor")) {

                logs::info(
                    "Open UI Editor requested "
                    "from SKSE Menu Framework.");

                if (!editor->Open()) {
                    logs::error(
                        "Failed to open "
                        "PrismaUI Editor "
                        "from menu.");
                }
                else {
                    logs::info(
                        "PrismaUI Editor opened "
                        "successfully.");

                    auto* mainWindow =
                        SKSEMenuFramework::
                            GetMainWindow();

                    if (mainWindow) {
                        mainWindow->
                            IsOpen.store(
                                false);

                        logs::info(
                            "SKSE Menu Framework "
                            "closed for "
                            "PrismaUI Editor.");
                    }
                    else {
                        logs::warn(
                            "Could not acquire "
                            "SKSE Menu Framework "
                            "main window.");
                    }
                }
            }
        }


        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::Spacing();


        // Save Settings

        if (ImGuiMCP::Button(
                "Save Settings")) {

            // Keep the currently active values so
            // they can be restored if saving fails.

            const float oldLossRatio =
                config->
                    GetLossRatio();

            const bool oldSleepRecovery =
                config->
                    IsSleepRecoveryEnabled();

            const float oldFullRecoveryHours =
                config->
                    GetFullRecoveryHours();

            const bool oldNaturalRegen =
                config->
                    IsNaturalHealthRegenerationEnabled();


            // Commit the pending menu values.

            config->
                SetLossRatio(
                    pendingLossRatio_);

            config->
                SetSleepRecoveryEnabled(
                    pendingSleepRecovery_);

            config->
                SetFullRecoveryHours(
                    pendingFullRecoveryHours_);

            config->
                SetNaturalHealthRegenerationEnabled(
                    pendingNaturalRegen_);


            // Save first. Gameplay-side effects are
            // applied only after persistence succeeds.

            if (config->Save()) {

                NaturalRegenController::
                    GetSingleton()->
                        Apply();

                // Reload the working copy from the
                // committed config.

                InitializePendingSettings();

                logs::info(
                    "Loss Gauge settings "
                    "saved and applied from menu.");
            }
            else {

                // Restore active configuration if the
                // TOML could not be written.

                config->
                    SetLossRatio(
                        oldLossRatio);

                config->
                    SetSleepRecoveryEnabled(
                        oldSleepRecovery);

                config->
                    SetFullRecoveryHours(
                        oldFullRecoveryHours);

                config->
                    SetNaturalHealthRegenerationEnabled(
                        oldNaturalRegen);

                NaturalRegenController::
                    GetSingleton()->
                        Apply();

                InitializePendingSettings();

                logs::error(
                    "Failed to save "
                    "Loss Gauge settings. "
                    "Previous settings restored.");
            }
        }


        ImGuiMCP::SameLine();


        // Reset Defaults

        if (ImGuiMCP::Button(
                "Reset Defaults")) {

            // Only change the menu working copy.
            // Nothing is applied or saved yet.

            ResetPendingSettings();

            logs::info(
                "Loss Gauge default settings "
                "loaded into menu. "
                "Press Save Settings to apply.");
        }
    }


    // Debug

    void __stdcall MenuManager::RenderDebug()
    {
        auto* config =
            ConfigManager::
                GetSingleton();

        auto* lossManager =
            LossManager::
                GetSingleton();


        // Runtime Status

        ImGuiMCP::Text(
            "Runtime Status");

        ImGuiMCP::Separator();

        if (!lossManager) {
            ImGuiMCP::Text(
                "Loss Gauge runtime state "
                "is unavailable.");
        }
        else {

            const float currentHealth =
                lossManager->
                    GetCurrentHealth();

            const float maxHealth =
                lossManager->
                    GetMaxHealth();

            const float recoverableHealth =
                lossManager->
                    GetRecoverableHealth();

            const float loss =
                lossManager->
                    GetLoss();

            const float recoveryBaseLoss =
                lossManager->
                    GetRecoveryBaseLoss();

            const float recoveryHours =
                lossManager->
                    GetRecoveryHours();


            float currentPct =
                0.0f;

            float recoverablePct =
                0.0f;

            float lossPct =
                0.0f;


            if (maxHealth > 0.0f) {
                currentPct =
                    (currentHealth /
                     maxHealth) *
                    100.0f;

                recoverablePct =
                    (recoverableHealth /
                     maxHealth) *
                    100.0f;

                lossPct =
                    (loss /
                     maxHealth) *
                    100.0f;
            }


            currentPct =
                std::clamp(
                    currentPct,
                    0.0f,
                    100.0f);

            recoverablePct =
                std::clamp(
                    recoverablePct,
                    0.0f,
                    100.0f);

            lossPct =
                std::clamp(
                    lossPct,
                    0.0f,
                    100.0f);


            char textBuffer[128]{};


            // Health

            ImGuiMCP::Text(
                "Health");

            ImGuiMCP::Spacing();

            std::snprintf(
                textBuffer,
                sizeof(textBuffer),
                "Current HP: %.2f / %.2f",
                currentHealth,
                maxHealth);

            ImGuiMCP::Text(
                textBuffer);

            std::snprintf(
                textBuffer,
                sizeof(textBuffer),
                "Current HP %%: %.2f%%",
                currentPct);

            ImGuiMCP::Text(
                textBuffer);

            ImGuiMCP::Spacing();

            std::snprintf(
                textBuffer,
                sizeof(textBuffer),
                "Recoverable HP: %.2f / %.2f",
                recoverableHealth,
                maxHealth);

            ImGuiMCP::Text(
                textBuffer);

            std::snprintf(
                textBuffer,
                sizeof(textBuffer),
                "Recoverable %%: %.2f%%",
                recoverablePct);

            ImGuiMCP::Text(
                textBuffer);

            ImGuiMCP::Spacing();


            // Loss

            ImGuiMCP::Text(
                "Loss");

            ImGuiMCP::Spacing();

            std::snprintf(
                textBuffer,
                sizeof(textBuffer),
                "Loss: %.2f",
                loss);

            ImGuiMCP::Text(
                textBuffer);

            std::snprintf(
                textBuffer,
                sizeof(textBuffer),
                "Loss %%: %.2f%%",
                lossPct);

            ImGuiMCP::Text(
                textBuffer);

            ImGuiMCP::Spacing();


            // Recovery

            ImGuiMCP::Text(
                "Recovery");

            ImGuiMCP::Spacing();

            std::snprintf(
                textBuffer,
                sizeof(textBuffer),
                "Recovery Base Loss: %.2f",
                recoveryBaseLoss);

            ImGuiMCP::Text(
                textBuffer);

            std::snprintf(
                textBuffer,
                sizeof(textBuffer),
                "Recovery Hours: %.2f h",
                recoveryHours);

            ImGuiMCP::Text(
                textBuffer);
        }


        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::Spacing();


        // Debug Options

        ImGuiMCP::Text(
            "Debug Options");

        ImGuiMCP::Separator();

        if (!config) {
            ImGuiMCP::Text(
                "Loss Gauge configuration "
                "is unavailable.");

            return;
        }

        bool debugLogging =
            config->
                IsDebugLoggingEnabled();

        if (ImGuiMCP::Checkbox(
                "Enable Debug Logging",
                &debugLogging)) {

            config->
                SetDebugLoggingEnabled(
                    debugLogging);
        }

        ImGuiMCP::Text(
            "Enables additional Loss Gauge "
            "diagnostic logging.");

        ImGuiMCP::Spacing();


        // Save Debug Setting

        if (ImGuiMCP::Button(
                "Save Debug Settings")) {

            if (config->Save()) {
                logs::info(
                    "Loss Gauge debug settings "
                    "saved from menu.");
            }
            else {
                logs::error(
                    "Failed to save "
                    "Loss Gauge debug settings "
                    "from menu.");
            }
        }
    }
}