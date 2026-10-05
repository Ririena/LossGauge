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

    ConfigManager* ConfigManager::GetSingleton()
    {
        static ConfigManager instance;
        return std::addressof(instance);
    }

    bool ConfigManager::Load()
    {
        ResetToDefaults();

        logs::info("================================");
        logs::info("Loss Gauge Configuration");
        logs::info("--------------------------------");

        try {
            const auto config =
                toml::parse_file(kConfigPath);

            if (const auto value =
                    config["Loss"]["LossRatio"]
                        .value<double>()) {

                SetLossRatio(
                    static_cast<float>(*value));
            }

            if (const auto value =
                    config["Sleep"]["EnableRecovery"]
                        .value<bool>()) {

                SetSleepRecoveryEnabled(*value);
            }

            if (const auto value =
                    config["Sleep"]["FullRecoveryHours"]
                        .value<double>()) {

                SetFullRecoveryHours(
                    static_cast<float>(*value));
            }

            if (const auto value =
                    config["Debug"]["EnableLogging"]
                        .value<bool>()) {

                SetDebugLoggingEnabled(*value);
            }
        }
        catch (const toml::parse_error& error) {
            logs::warn(
                "Could not load config: {}",
                error.description());

            logs::warn(
                "Using default configuration.");

            logs::info(
                "Loss Ratio:          {:.2f}",
                lossRatio_);

            logs::info(
                "Sleep Recovery:      {}",
                sleepRecoveryEnabled_ ?
                    "Enabled" :
                    "Disabled");

            logs::info(
                "Full Recovery Hours: {:.2f}",
                fullRecoveryHours_);

            logs::info(
                "Debug Logging:       {}",
                debugLoggingEnabled_ ?
                    "Enabled" :
                    "Disabled");

            logs::info("================================");

            return false;
        }

        logs::info(
            "Loss Ratio:          {:.2f} ({:.0f}%)",
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
            "Debug Logging:       {}",
            debugLoggingEnabled_ ?
                "Enabled" :
                "Disabled");

        logs::info("================================");

        return true;
    }

    bool ConfigManager::Save() const
    {
        try {
            toml::table config;

            config.insert(
                "Loss",
                toml::table{
                    {
                        "LossRatio",
                        lossRatio_
                    }
                });

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

            config.insert(
                "Debug",
                toml::table{
                    {
                        "EnableLogging",
                        debugLoggingEnabled_
                    }
                });

            std::ofstream file(kConfigPath);

            if (!file.is_open()) {
                logs::error(
                    "Failed to open config for writing: {}",
                    kConfigPath);

                return false;
            }

            file << config;

            if (!file.good()) {
                logs::error(
                    "Failed while writing config: {}",
                    kConfigPath);

                return false;
            }

            logs::info(
                "Loss Gauge configuration saved.");

            return true;
        }
        catch (const std::exception& error) {
            logs::error(
                "Failed to save configuration: {}",
                error.what());

            return false;
        }
    }

    float ConfigManager::GetLossRatio() const
    {
        return lossRatio_;
    }

    bool ConfigManager::IsSleepRecoveryEnabled() const
    {
        return sleepRecoveryEnabled_;
    }

    float ConfigManager::GetFullRecoveryHours() const
    {
        return fullRecoveryHours_;
    }

    bool ConfigManager::IsDebugLoggingEnabled() const
    {
        return debugLoggingEnabled_;
    }

    void ConfigManager::SetLossRatio(float a_value)
    {
        if (!std::isfinite(a_value)) {
            logs::warn(
                "Invalid LossRatio. "
                "Using default value.");

            lossRatio_ =
                kDefaultLossRatio;

            return;
        }

        lossRatio_ = std::clamp(
            a_value,
            kMinLossRatio,
            kMaxLossRatio);
    }

    void ConfigManager::SetSleepRecoveryEnabled(
        bool a_enabled)
    {
        sleepRecoveryEnabled_ =
            a_enabled;
    }

    void ConfigManager::SetFullRecoveryHours(
        float a_hours)
    {
        if (!std::isfinite(a_hours)) {
            logs::warn(
                "Invalid FullRecoveryHours. "
                "Using default value.");

            fullRecoveryHours_ =
                kDefaultFullRecoveryHours;

            return;
        }

        fullRecoveryHours_ = std::clamp(
            a_hours,
            kMinFullRecoveryHours,
            kMaxFullRecoveryHours);
    }

    void ConfigManager::SetDebugLoggingEnabled(
        bool a_enabled)
    {
        debugLoggingEnabled_ =
            a_enabled;
    }

    void ConfigManager::ResetToDefaults()
    {
        lossRatio_ =
            kDefaultLossRatio;

        sleepRecoveryEnabled_ =
            kDefaultSleepRecoveryEnabled;

        fullRecoveryHours_ =
            kDefaultFullRecoveryHours;

        debugLoggingEnabled_ =
            kDefaultDebugLoggingEnabled;
    }
}