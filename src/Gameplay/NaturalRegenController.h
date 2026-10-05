#pragma once

namespace LossGauge
{
    class NaturalRegenController
    {
    public:
        static NaturalRegenController*
        GetSingleton();

        // Apply the current configuration.
        //
        // EnableNaturalRegeneration = false:
        // suppress natural Health regeneration.
        //
        // EnableNaturalRegeneration = true:
        // allow normal Skyrim Health regeneration.
        void Apply();

        // Remove the temporary modifier applied
        // by Loss Gauge during this runtime.
        void Restore();

        // Clear DLL-side state only.
        //
        // Does NOT modify Skyrim ActorValues.
        void ResetState();

        [[nodiscard]]
        bool IsSuppressionApplied() const;

    private:
        NaturalRegenController() =
            default;

        NaturalRegenController(
            const NaturalRegenController&) =
            delete;

        NaturalRegenController(
            NaturalRegenController&&) =
            delete;

        NaturalRegenController&
        operator=(
            const NaturalRegenController&) =
            delete;

        NaturalRegenController&
        operator=(
            NaturalRegenController&&) =
            delete;

        // True only when Loss Gauge has applied
        // its temporary HealRateMult modifier
        // during the current runtime.
        bool suppressionApplied_{
            false
        };

        // Exact modifier applied by Loss Gauge.
        //
        // Example:
        //
        // original effective HealRateMult = 100
        // appliedModifier_ = -100
        //
        // Restore() reverses it with +100.
        float appliedModifier_{
            0.0f
        };
    };
}