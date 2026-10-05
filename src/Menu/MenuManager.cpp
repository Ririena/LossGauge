#include "Menu/MenuManager.h"

#include "Config/ConfigManager.h"
#include "External/SKSEMenuFramework.h"
#include "Gameplay/NaturalRegenController.h"

namespace LossGauge
{
    bool MenuManager::registered_ = false;

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

        // Root section shown in the
        // SKSE Menu Framework menu.
        SKSEMenuFramework::
            SetSection(
                "Loss Gauge NG");

        SKSEMenuFramework::
            AddSectionItem(
                "Settings",
                RenderSettings);

        registered_ = true;

        logs::info(
            "Loss Gauge NG menu registered.");
    }

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

        // ========================================
        // Loss
        // ========================================

        ImGuiMCP::Text("Loss");

        ImGuiMCP::Separator();

        float lossRatio =
            config->GetLossRatio();

        if (ImGuiMCP::SliderFloat(
                "Loss Ratio",
                &lossRatio,
                0.0f,
                1.0f,
                "%.2f")) {

            config->
                SetLossRatio(
                    lossRatio);
        }

        ImGuiMCP::Text(
            "Percentage of incoming damage "
            "converted into Loss.");

        ImGuiMCP::Spacing();

        // ========================================
        // Sleep Recovery
        // ========================================

        ImGuiMCP::Text(
            "Sleep Recovery");

        ImGuiMCP::Separator();

        bool sleepRecovery =
            config->
                IsSleepRecoveryEnabled();

        if (ImGuiMCP::Checkbox(
                "Enable Sleep Recovery",
                &sleepRecovery)) {

            config->
                SetSleepRecoveryEnabled(
                    sleepRecovery);
        }

        float fullRecoveryHours =
            config->
                GetFullRecoveryHours();

        if (ImGuiMCP::SliderFloat(
                "Full Recovery Hours",
                &fullRecoveryHours,
                1.0f,
                24.0f,
                "%.1f h")) {

            config->
                SetFullRecoveryHours(
                    fullRecoveryHours);
        }

        ImGuiMCP::Text(
            "Cumulative sleep time required "
            "to fully recover Loss.");

        ImGuiMCP::Spacing();

        // ========================================
        // Health
        // ========================================

        ImGuiMCP::Text("Health");

        ImGuiMCP::Separator();

        bool naturalRegen =
            config->
                IsNaturalHealthRegenerationEnabled();

        if (ImGuiMCP::Checkbox(
                "Natural Health Regeneration",
                &naturalRegen)) {

            config->
                SetNaturalHealthRegenerationEnabled(
                    naturalRegen);

            // Apply immediately to the
            // current runtime.
            NaturalRegenController::
                GetSingleton()->
                Apply();
        }

        ImGuiMCP::Text(
            "Controls Skyrim's natural "
            "health regeneration.");

        ImGuiMCP::Spacing();

        // ========================================
        // Debug
        // ========================================

        ImGuiMCP::Text("Debug");

        ImGuiMCP::Separator();

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

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::Spacing();

        // ========================================
        // Save
        // ========================================

        if (ImGuiMCP::Button(
                "Save Settings")) {

            if (config->Save()) {
                logs::info(
                    "Loss Gauge settings "
                    "saved from menu.");
            }
            else {
                logs::error(
                    "Failed to save "
                    "Loss Gauge settings "
                    "from menu.");
            }
        }

        ImGuiMCP::SameLine();

        // ========================================
        // Reset
        // ========================================

        if (ImGuiMCP::Button(
                "Reset Defaults")) {

            config->
                ResetToDefaults();

            // Make runtime regen state match
            // the newly reset configuration.
            NaturalRegenController::
                GetSingleton()->
                Apply();

            if (config->Save()) {
                logs::info(
                    "Loss Gauge settings "
                    "reset to defaults.");
            }
            else {
                logs::error(
                    "Loss Gauge defaults "
                    "could not be saved.");
            }
        }
    }
}