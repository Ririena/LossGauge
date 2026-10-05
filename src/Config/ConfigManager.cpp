#include "Config/ConfigManager.h"

#include <fstream>
#include <toml++/toml.hpp>

namespace LossGauge
{
    namespace
    {
        constexpr auto kConfigPath =
            "Data/SKSE/Plugins/LossGauge.toml";
    }

    ConfigManager*
    ConfigManager::GetSingleton()
    {
        static ConfigManager instance;
        return std::addressof(instance);
    }

    bool ConfigManager::Load()
    {
        // Always start from known defaults.
        ResetToDefaults();

        try {
            const auto config =
                toml::parse_file(kConfigPath);

            // ========================================
            // [Loss]
            // ========================================

            if (const auto value =
                    config["Loss"]
                          ["LossRatio"]
                              .value<double>()) {

                lossRatio_ =
                    std::clamp(
                        static_cast<float>(*value),
                        kMinLossRatio,
                        kMaxLossRatio);
            }

            // ========================================
            // [Sleep]
            // ========================================

            if (const auto value =
                    config["Sleep"]
                          ["EnableRecovery"]
                              .value<bool>()) {

                sleepRecoveryEnabled_ =
                    *value;
            }

            if (const auto value =
                    config["Sleep"]
                          ["FullRecoveryHours"]
                              .value<double>()) {

                fullRecoveryHours_ =
                    std::clamp(
                        static_cast<float>(*value),
                        kMinFullRecoveryHours,
                        kMaxFullRecoveryHours);
            }

            // ========================================
            // [Health]
            // ========================================

            if (const auto value =
                    config["Health"]
                          ["EnableNaturalRegeneration"]
                              .value<bool>()) {

                naturalHealthRegenerationEnabled_ =
                    *value;
            }

            // ========================================
            // [Debug]
            // ========================================

            if (const auto value =
                    config["Debug"]
                          ["EnableLogging"]
                              .value<bool>()) {

                debugLoggingEnabled_ =
                    *value;
            }

            // ========================================
            // Startup log
            // ========================================

            logs::info(
                "================================");

            logs::info(
                "Loss Gauge Configuration");

            logs::info(
                "--------------------------------");

            logs::info(
                "Loss Ratio:          "
                "{:.2f} ({:.0f}%)",
                lossRatio_,
                lossRatio_ * 100.0f);

            logs::info(
                "Sleep Recovery:      {}",
                sleepRecoveryEnabled_ ?
                    "Enabled" :
                    "Disabled");

            logs::info(
                "Full Recovery Hours: {:.2f}",
                fullRecoveryHours_);

            logs::info(
                "Natural HP Regen:    {}",
                naturalHealthRegenerationEnabled_ ?
                    "Enabled" :
                    "Disabled");

            logs::info(
                "Debug Logging:       {}",
                debugLoggingEnabled_ ?
                    "Enabled" :
                    "Disabled");

            logs::info(
                "================================");

            return true;
        }
        catch (const toml::parse_error& e) {
            logs::error(
                "Failed to parse LossGauge.toml: {}",
                e.description());

            logs::warn(
                "Using default configuration.");

            return false;
        }
        catch (const std::exception& e) {
            logs::error(
                "Failed to load LossGauge.toml: {}",
                e.what());

            logs::warn(
                "Using default configuration.");

            return false;
        }
    }

    bool ConfigManager::Save() const
    {
        try {
            toml::table config;

            // ========================================
            // [Loss]
            // ========================================

            config.insert(
                "Loss",
                toml::table{
                    {
                        "LossRatio",
                        lossRatio_
                    }
                });

            // ========================================
            // [Sleep]
            // ========================================

            config.insert(
                "Sleep",
                toml::table{
                    {
                        "EnableRecovery",
                        sleepRecoveryEnabled_
                    },
                    {
                        "FullRecoveryHours",
                        fullRecoveryHours_
                    }
                });

            // ========================================
            // [Health]
            // ========================================

            config.insert(
                "Health",
                toml::table{
                    {
                        "EnableNaturalRegeneration",
                        naturalHealthRegenerationEnabled_
                    }
                });

            // ========================================
            // [Debug]
            // ========================================

            config.insert(
                "Debug",
                toml::table{
                    {
                        "EnableLogging",
                        debugLoggingEnabled_
                    }
                });

            std::ofstream file(
                kConfigPath,
                std::ios::out |
                std::ios::trunc);

            if (!file.is_open()) {
                logs::error(
                    "Failed to open "
                    "LossGauge.toml for writing.");

                return false;
            }

            file << config;

            if (!file.good()) {
                logs::error(
                    "Failed while writing "
                    "LossGauge.toml.");

                return false;
            }

            logs::info(
                "LossGauge.toml saved.");

            return true;
        }
        catch (const std::exception& e) {
            logs::error(
                "Failed to save LossGauge.toml: {}",
                e.what());

            return false;
        }
    }

    // ============================================
    // Getters
    // ============================================

    float ConfigManager::
    GetLossRatio() const
    {
        return lossRatio_;
    }

    bool ConfigManager::
    IsSleepRecoveryEnabled() const
    {
        return sleepRecoveryEnabled_;
    }

    float ConfigManager::
    GetFullRecoveryHours() const
    {
        return fullRecoveryHours_;
    }

    bool ConfigManager::
    IsNaturalHealthRegenerationEnabled() const
    {
        return naturalHealthRegenerationEnabled_;
    }

    bool ConfigManager::
    IsDebugLoggingEnabled() const
    {
        return debugLoggingEnabled_;
    }

    // ============================================
    // Setters
    // ============================================

    void ConfigManager::
    SetLossRatio(
        float a_value)
    {
        lossRatio_ =
            std::clamp(
                a_value,
                kMinLossRatio,
                kMaxLossRatio);
    }

    void ConfigManager::
    SetSleepRecoveryEnabled(
        bool a_enabled)
    {
        sleepRecoveryEnabled_ =
            a_enabled;
    }

    void ConfigManager::
    SetFullRecoveryHours(
        float a_hours)
    {
        fullRecoveryHours_ =
            std::clamp(
                a_hours,
                kMinFullRecoveryHours,
                kMaxFullRecoveryHours);
    }

    void ConfigManager::
    SetNaturalHealthRegenerationEnabled(
        bool a_enabled)
    {
        naturalHealthRegenerationEnabled_ =
            a_enabled;
    }

    void ConfigManager::
    SetDebugLoggingEnabled(
        bool a_enabled)
    {
        debugLoggingEnabled_ =
            a_enabled;
    }

    // ============================================
    // Defaults
    // ============================================

    void ConfigManager::
    ResetToDefaults()
    {
        lossRatio_ =
            kDefaultLossRatio;

        sleepRecoveryEnabled_ =
            kDefaultSleepRecoveryEnabled;

        fullRecoveryHours_ =
            kDefaultFullRecoveryHours;

        naturalHealthRegenerationEnabled_ =
            kDefaultNaturalHealthRegenerationEnabled;

        debugLoggingEnabled_ =
            kDefaultDebugLoggingEnabled;
    }
}