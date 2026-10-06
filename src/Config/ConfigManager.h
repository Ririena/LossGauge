#pragma once

#include "UI/UIConfig.h"

namespace LossGauge
{
    class ConfigManager
    {
    public:
        static ConfigManager* GetSingleton();

        bool Load();
        bool Save() const;

        // ========================================
        // Gameplay Getters
        // ========================================

        [[nodiscard]]
        float GetLossRatio() const;

        [[nodiscard]]
        bool IsSleepRecoveryEnabled() const;

        [[nodiscard]]
        float GetFullRecoveryHours() const;

        [[nodiscard]]
        bool IsNaturalHealthRegenerationEnabled() const;

        [[nodiscard]]
        bool IsDebugLoggingEnabled() const;

        // ========================================
        // UI Getter
        // ========================================

        [[nodiscard]]
        const UIConfig& GetUIConfig() const;

        // ========================================
        // Gameplay Setters
        // ========================================

        void SetLossRatio(
            float a_value);

        void SetSleepRecoveryEnabled(
            bool a_enabled);

        void SetFullRecoveryHours(
            float a_hours);

        void SetNaturalHealthRegenerationEnabled(
            bool a_enabled);

        void SetDebugLoggingEnabled(
            bool a_enabled);

        // ========================================
        // UI Setter
        // ========================================

        void SetUIConfig(
            const UIConfig& a_config);

        // ========================================
        // Defaults
        // ========================================

        void ResetToDefaults();

    private:
        ConfigManager() = default;

        ConfigManager(
            const ConfigManager&) = delete;

        ConfigManager(
            ConfigManager&&) = delete;

        ConfigManager& operator=(
            const ConfigManager&) = delete;

        ConfigManager& operator=(
            ConfigManager&&) = delete;

        // ========================================
        // Gameplay Defaults
        // ========================================

        static constexpr float
            kDefaultLossRatio =
                0.10f;

        static constexpr bool
            kDefaultSleepRecoveryEnabled =
                true;

        static constexpr float
            kDefaultFullRecoveryHours =
                8.0f;

        static constexpr bool
            kDefaultNaturalHealthRegenerationEnabled =
                false;

        static constexpr bool
            kDefaultDebugLoggingEnabled =
                false;

        // ========================================
        // Gameplay Limits
        // ========================================

        static constexpr float
            kMinLossRatio =
                0.0f;

        static constexpr float
            kMaxLossRatio =
                1.0f;

        static constexpr float
            kMinFullRecoveryHours =
                1.0f;

        static constexpr float
            kMaxFullRecoveryHours =
                24.0f;

        // ========================================
        // Gameplay State
        // ========================================

        float lossRatio_{
            kDefaultLossRatio
        };

        bool sleepRecoveryEnabled_{
            kDefaultSleepRecoveryEnabled
        };

        float fullRecoveryHours_{
            kDefaultFullRecoveryHours
        };

        bool naturalHealthRegenerationEnabled_{
            kDefaultNaturalHealthRegenerationEnabled
        };

        bool debugLoggingEnabled_{
            kDefaultDebugLoggingEnabled
        };

        // ========================================
        // UI State
        // ========================================

        UIConfig uiConfig_{};
    };
}