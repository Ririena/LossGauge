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



        return std::addressof(

            instance);

    }



    bool ConfigManager::Load()

    {

        // Always start from known defaults.

        ResetToDefaults();



        try {

            const auto config =

                toml::parse_file(

                    kConfigPath);



            // [Loss]



            if (const auto value =

                    config["Loss"]

                          ["LossRatio"]

                              .value<double>()) {



                lossRatio_ =

                    std::clamp(

                        static_cast<float>(

                            *value),

                        kMinLossRatio,

                        kMaxLossRatio);

            }



            // [Sleep]



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

                        static_cast<float>(

                            *value),

                        kMinFullRecoveryHours,

                        kMaxFullRecoveryHours);

            }



            // [Health]



            if (const auto value =

                    config["Health"]

                          ["EnableNaturalRegeneration"]

                              .value<bool>()) {



                naturalHealthRegenerationEnabled_ =

                    *value;

            }



            // [Debug]



            if (const auto value =

                    config["Debug"]

                          ["EnableLogging"]

                              .value<bool>()) {



                debugLoggingEnabled_ =

                    *value;

            }



            // [UI]



            if (const auto value =

                    config["UI"]

                          ["PositionX"]

                              .value<double>()) {



                uiConfig_.positionX =

                    static_cast<float>(

                        *value);

            }



            if (const auto value =

                    config["UI"]

                          ["PositionY"]

                              .value<double>()) {



                uiConfig_.positionY =

                    static_cast<float>(

                        *value);

            }



            if (const auto value =

                    config["UI"]

                          ["Width"]

                              .value<double>()) {



                uiConfig_.width =

                    static_cast<float>(

                        *value);

            }



            if (const auto value =

                    config["UI"]

                          ["Height"]

                              .value<double>()) {



                uiConfig_.height =

                    static_cast<float>(

                        *value);

            }



            if (const auto value =

                    config["UI"]

                          ["ColorR"]

                              .value<std::int64_t>()) {



                uiConfig_.colorR =

                    static_cast<std::uint8_t>(

                        std::clamp<std::int64_t>(

                            *value,

                            0,

                            255));

            }



            if (const auto value =

                    config["UI"]

                          ["ColorG"]

                              .value<std::int64_t>()) {



                uiConfig_.colorG =

                    static_cast<std::uint8_t>(

                        std::clamp<std::int64_t>(

                            *value,

                            0,

                            255));

            }



            if (const auto value =

                    config["UI"]

                          ["ColorB"]

                              .value<std::int64_t>()) {



                uiConfig_.colorB =

                    static_cast<std::uint8_t>(

                        std::clamp<std::int64_t>(

                            *value,

                            0,

                            255));

            }



            if (const auto value =

                    config["UI"]

                          ["Opacity"]

                              .value<double>()) {



                uiConfig_.opacity =

                    static_cast<float>(

                        *value);

            }



            if (const auto value = config["UI"]["BorderRadius"].value<double>()) {
                uiConfig_.borderRadius = static_cast<float>(*value);
            }

            if (const auto value = config["UI"]["EnableBorder"].value<bool>()) {
                uiConfig_.enableBorder = *value;
            }

            if (const auto value = config["UI"]["BorderWidth"].value<double>()) {
                uiConfig_.borderWidth = static_cast<float>(*value);
            }

            if (const auto value = config["UI"]["BorderColorR"].value<std::int64_t>()) {
                uiConfig_.borderColorR = static_cast<std::uint8_t>(std::clamp<std::int64_t>(*value, 0, 255));
            }

            if (const auto value = config["UI"]["BorderColorG"].value<std::int64_t>()) {
                uiConfig_.borderColorG = static_cast<std::uint8_t>(std::clamp<std::int64_t>(*value, 0, 255));
            }

            if (const auto value = config["UI"]["BorderColorB"].value<std::int64_t>()) {
                uiConfig_.borderColorB = static_cast<std::uint8_t>(std::clamp<std::int64_t>(*value, 0, 255));
            }

            if (const auto value = config["UI"]["BorderOpacity"].value<double>()) {
                uiConfig_.borderOpacity = static_cast<float>(*value);
            }

            if (const auto value =

                    config["UI"]

                          ["EnableAnimation"]

                              .value<bool>()) {



                uiConfig_.enableAnimation =

                    *value;

            }



            if (const auto value =

                    config["UI"]

                          ["AnimationDuration"]

                              .value<double>()) {



                uiConfig_.animationDuration =

                    static_cast<float>(

                        *value);

            }



            // Validate all UI values after loading.



            uiConfig_.Clamp();



            // Startup Log



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

                "--------------------------------");



            logs::info(

                "UI Position:         "

                "X {:.1f}, Y {:.1f}",

                uiConfig_.positionX,

                uiConfig_.positionY);



            logs::info(

                "UI Size:             "

                "{:.1f} x {:.1f}",

                uiConfig_.width,

                uiConfig_.height);



            logs::info(

                "UI Color:            "

                "RGB({}, {}, {})",

                static_cast<int>(

                    uiConfig_.colorR),

                static_cast<int>(

                    uiConfig_.colorG),

                static_cast<int>(

                    uiConfig_.colorB));



            logs::info(

                "UI Opacity:          {:.2f}",

                uiConfig_.opacity);



            logs::info(

                "UI Animation:        {}",

                uiConfig_.enableAnimation ?

                    "Enabled" :

                    "Disabled");



            logs::info(

                "UI Animation Time:   {:.2f}s",

                uiConfig_.animationDuration);



            logs::info(

                "================================");



            return true;

        }

        catch (const toml::parse_error& e) {

            logs::error(

                "Failed to parse "

                "LossGauge.toml: {}",

                e.description());



            logs::warn(

                "Using default configuration.");



            return false;

        }

        catch (const std::exception& e) {

            logs::error(

                "Failed to load "

                "LossGauge.toml: {}",

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



            // [Loss]



            config.insert(

                "Loss",

                toml::table{

                    {

                        "LossRatio",

                        lossRatio_

                    }

                });



            // [Sleep]



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



            // [Health]



            config.insert(

                "Health",

                toml::table{

                    {

                        "EnableNaturalRegeneration",

                        naturalHealthRegenerationEnabled_

                    }

                });



            // [Debug]



            config.insert(

                "Debug",

                toml::table{

                    {

                        "EnableLogging",

                        debugLoggingEnabled_

                    }

                });



            // [UI]



            config.insert(

                "UI",

                toml::table{

                    {

                        "PositionX",

                        uiConfig_.positionX

                    },

                    {

                        "PositionY",

                        uiConfig_.positionY

                    },

                    {

                        "Width",

                        uiConfig_.width

                    },

                    {

                        "Height",

                        uiConfig_.height

                    },

                    {

                        "ColorR",

                        static_cast<std::int64_t>(

                            uiConfig_.colorR)

                    },

                    {

                        "ColorG",

                        static_cast<std::int64_t>(

                            uiConfig_.colorG)

                    },

                    {

                        "ColorB",

                        static_cast<std::int64_t>(

                            uiConfig_.colorB)

                    },

                    {

                        "Opacity",

                        uiConfig_.opacity

                    },

                    {
                        "BorderRadius",
                        uiConfig_.borderRadius
                    },
                    {
                        "EnableBorder",
                        uiConfig_.enableBorder
                    },
                    {
                        "BorderWidth",
                        uiConfig_.borderWidth
                    },
                    {
                        "BorderColorR",
                        static_cast<std::int64_t>(uiConfig_.borderColorR)
                    },
                    {
                        "BorderColorG",
                        static_cast<std::int64_t>(uiConfig_.borderColorG)
                    },
                    {
                        "BorderColorB",
                        static_cast<std::int64_t>(uiConfig_.borderColorB)
                    },
                    {
                        "BorderOpacity",
                        uiConfig_.borderOpacity
                    },
                    {

                        "EnableAnimation",

                        uiConfig_.enableAnimation

                    },

                    {

                        "AnimationDuration",

                        uiConfig_.animationDuration

                    }

                });



            // Write File



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

                "Failed to save "

                "LossGauge.toml: {}",

                e.what());



            return false;

        }

    }



    // Gameplay Getters



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



    // UI Getter



    const UIConfig&

    ConfigManager::GetUIConfig() const

    {

        return uiConfig_;

    }



    // Gameplay Setters



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



    // UI Setter



    void ConfigManager::

    SetUIConfig(

        const UIConfig& a_config)

    {

        uiConfig_ =

            a_config;



        uiConfig_.Clamp();

    }



    // Defaults



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



        // UIConfig's member initializers contain

        // the canonical UI defaults.

        uiConfig_ =

            UIConfig{};



        uiConfig_.Clamp();

    }

}