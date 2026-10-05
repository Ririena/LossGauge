#pragma once

namespace LossGauge
{
    class ConfigManager
    {
    public:
        static ConfigManager* GetSingleton();

        bool Load();
        bool Save() const;

        [[nodiscard]] float GetLossRatio() const;
        [[nodiscard]] bool IsSleepRecoveryEnabled() const;
        [[nodiscard]] float GetFullRecoveryHours() const;
        [[nodiscard]] bool IsDebugLoggingEnabled() const;

        void SetLossRatio(float a_value);
        void SetSleepRecoveryEnabled(bool a_enabled);
        void SetFullRecoveryHours(float a_hours);
        void SetDebugLoggingEnabled(bool a_enabled);

        void ResetToDefaults();

    private:
        ConfigManager() = default;

        ConfigManager(const ConfigManager&) = delete;
        ConfigManager(ConfigManager&&) = delete;

        ConfigManager& operator=(
            const ConfigManager&) = delete;

        ConfigManager& operator=(
            ConfigManager&&) = delete;

        static constexpr float kDefaultLossRatio = 0.10f;
        static constexpr bool kDefaultSleepRecoveryEnabled = true;
        static constexpr float kDefaultFullRecoveryHours = 8.0f;
        static constexpr bool kDefaultDebugLoggingEnabled = false;

        static constexpr float kMinLossRatio = 0.0f;
        static constexpr float kMaxLossRatio = 1.0f;

        static constexpr float kMinFullRecoveryHours = 1.0f;
        static constexpr float kMaxFullRecoveryHours = 24.0f;

        float lossRatio_{ kDefaultLossRatio };

        bool sleepRecoveryEnabled_{
            kDefaultSleepRecoveryEnabled
        };

        float fullRecoveryHours_{
            kDefaultFullRecoveryHours
        };

        bool debugLoggingEnabled_{
            kDefaultDebugLoggingEnabled
        };
    };
}